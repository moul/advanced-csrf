package main

import (
	"fmt"
	"log"
	"net"
	"net/http"
	"strings"
	"time"
)

type AcsrfClient struct {
	Sockets map[string]http.ConnState
}

type AcsrfServer struct {
	Server  *http.Server
	Clients map[string]AcsrfClient
}

func (s *AcsrfServer) Handler(w http.ResponseWriter, r *http.Request) {
	//fmt.Printf("Handler: w=%q, r=%v s=%q\n", w, r, s)
	w.Write([]byte("HELLO"))
}

func (s *AcsrfServer) ConnState(conn net.Conn, state http.ConnState) {
	remoteAddr := conn.RemoteAddr().String()
	remoteAddrParts := strings.Split(remoteAddr, ":")
	remoteHost := remoteAddrParts[0]
	remotePort := remoteAddrParts[1]

	if _, ok := s.Clients[remoteHost]; !ok {
		s.Clients[remoteHost] = AcsrfClient{
			Sockets: make(map[string]http.ConnState),
		}
	}

	if state == http.StateClosed {
		delete(s.Clients[remoteHost].Sockets, remotePort)
	} else {
		s.Clients[remoteHost].Sockets[remotePort] = state
	}

	fmt.Printf("%s: %q\n", remoteHost, s.Clients[remoteHost])
}

func New() *AcsrfServer {
	acsrf := AcsrfServer{
		Clients: make(map[string]AcsrfClient),
		Server: &http.Server{
			Addr:           ":8080",
			ReadTimeout:    10 * time.Second,
			WriteTimeout:   10 * time.Second,
			MaxHeaderBytes: 1 << 20,
		},
	}
	acsrf.Server.Handler = http.HandlerFunc(acsrf.Handler)
	acsrf.Server.ConnState = acsrf.ConnState
	return &acsrf
}

func main() {
	acsrf := New()

	log.Fatal(acsrf.Server.ListenAndServe())
}
