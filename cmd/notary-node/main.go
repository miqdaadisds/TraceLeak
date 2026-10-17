// TraceLeak Notary Quorum Validator Node (Go Microservice)
// Ministry of Defence / WESEE (SIH 2026 - PS 237)
// Provides lightweight Byzantine-tolerant consensus monitoring for air-gapped enclaves.

package main

import (
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"sync"
	"time"
)

type DecryptionReceipt struct {
	ReceiptID           string  `json:"receipt_id"`
	DocID               string  `json:"doc_id"`
	RecipientID         string  `json:"recipient_id"`
	SessionID           string  `json:"session_id"`
	WatermarkID         string  `json:"watermark_id"`
	Timestamp           float64 `json:"timestamp"`
	RecipientSignature  string  `json:"recipient_signature_b64"`
	EventDigest         string  `json:"event_digest"`
}

type QuorumEndorsement struct {
	ValidatorID string `json:"validator_id"`
	Signature   string `json:"signature"`
	Timestamp   int64  `json:"timestamp"`
}

type NotaryServer struct {
	mu           sync.Mutex
	nodeID       string
	endorsements map[string][]QuorumEndorsement
}

func (s *NotaryServer) handleAudit(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method Not Allowed", http.StatusMethodNotAllowed)
		return
	}

	var receipt DecryptionReceipt
	if err := json.NewDecoder(r.Body).Decode(&receipt); err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

	s.mu.Lock()
	defer s.mu.Unlock()

	hasher := sha256.New()
	hasher.Write([]byte(receipt.EventDigest + s.nodeID))
	nodeSig := hex.EncodeToString(hasher.Sum(nil))

	endorsement := QuorumEndorsement{
		ValidatorID: s.nodeID,
		Signature:   nodeSig,
		Timestamp:   time.Now().Unix(),
	}

	s.endorsements[receipt.ReceiptID] = append(s.endorsements[receipt.ReceiptID], endorsement)

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(map[string]interface{}{
		"status":      "ENDORSED",
		"validator":   s.nodeID,
		"endorsement": endorsement,
	})
}

func main() {
	server := &NotaryServer{
		nodeID:       "NODE-GO-01",
		endorsements: make(map[string][]QuorumEndorsement),
	}

	http.HandleFunc("/api/notary/endorse", server.handleAudit)
	http.HandleFunc("/health", func(w http.ResponseWriter, r *http.Request) {
		w.Write([]byte("TRACELEAK-NOTARY-OK"))
	})

	fmt.Println("TraceLeak Go Notary Quorum Validator Node listening on :8090...")
	log.Fatal(http.ListenAndServe("127.0.0.1:8090", nil))
}
