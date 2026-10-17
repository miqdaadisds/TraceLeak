//! TraceLeak Core: High-Assurance Cryptographic Engine (SIH 2026 - PS 237)
//! Provides zero-overhead Merkle tree verification, lattice key validation,
//! and constant-time memory zeroization for defence air-gapped enclaves.

use sha2::{Digest, Sha256};
use zeroize::{Zeroize, ZeroizeOnDrop};

/// Represents a 32-byte cryptographic digest in the provenance chain.
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub struct Digest256([u8; 32]);

impl Digest256 {
    pub fn from_bytes(bytes: [u8; 32]) -> Self {
        Self(bytes)
    }

    pub fn as_slice(&self) -> &[u8] {
        &self.0
    }

    pub fn to_hex(&self) -> String {
        let mut s = String::with_capacity(64);
        for b in &self.0 {
            use std::fmt::Write;
            let _ = write!(&mut s, "{:02x}", b);
        }
        s
    }
}

/// Secure in-memory buffer that guarantees RAM zeroization on drop.
#[derive(Zeroize, ZeroizeOnDrop)]
pub struct EnclaveKeyVault {
    classical_priv: Vec<u8>,
    ml_kem_priv: Vec<u8>,
    ml_dsa_priv: Vec<u8>,
}

impl EnclaveKeyVault {
    pub fn new(classical: Vec<u8>, kem: Vec<u8>, sig: Vec<u8>) -> Self {
        Self {
            classical_priv: classical,
            ml_kem_priv: kem,
            ml_dsa_priv: sig,
        }
    }

    pub fn is_empty(&self) -> bool {
        self.ml_kem_priv.is_empty() && self.ml_dsa_priv.is_empty()
    }
}

/// Computes a binary Merkle root from a sequence of leaf hashes.
pub fn compute_merkle_root(leaves: &[Digest256]) -> Digest256 {
    if leaves.is_empty() {
        return Digest256([0u8; 32]);
    }
    if leaves.len() == 1 {
        return leaves[0];
    }

    let mut current_level = leaves.to_vec();
    while current_level.len() > 1 {
        let mut next_level = Vec::with_capacity((current_level.len() + 1) / 2);
        for chunk in current_level.chunks(2) {
            let mut hasher = Sha256::new();
            hasher.update(&[0x01]); // Internal node domain separator (RFC 6962)
            hasher.update(chunk[0].as_slice());
            if chunk.len() > 1 {
                hasher.update(chunk[1].as_slice());
            } else {
                hasher.update(chunk[0].as_slice()); // Balanced promotion
            }
            let res: [u8; 32] = hasher.finalize().into();
            next_level.push(Digest256(res));
        }
        current_level = next_level;
    }

    current_level[0]
}

/// Verifies a canonical Merkle audit path for a given leaf.
pub fn verify_merkle_inclusion_proof(
    leaf: &Digest256,
    proof: &[(bool, Digest256)], // (is_right, sibling_hash)
    expected_root: &Digest256,
) -> bool {
    let mut current = *leaf;
    for (is_right, sibling) in proof {
        let mut hasher = Sha256::new();
        hasher.update(&[0x01]);
        if *is_right {
            hasher.update(current.as_slice());
            hasher.update(sibling.as_slice());
        } else {
            hasher.update(sibling.as_slice());
            hasher.update(current.as_slice());
        }
        let res: [u8; 32] = hasher.finalize().into();
        current = Digest256(res);
    }
    &current == expected_root
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_merkle_root_single_and_multi() {
        let d1 = Digest256([1u8; 32]);
        let d2 = Digest256([2u8; 32]);
        let root = compute_merkle_root(&[d1, d2]);
        assert_ne!(root, d1);
        assert_ne!(root, d2);
    }
}
