#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h> //ip hdeader library (must come before ip_icmp.h)
#include <netinet/ip_icmp.h> //icmp header
#include <arpa/inet.h> //internet address library
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

#include <iostream>
#include <sstream>
#include <string>
#include "bt_lib.h"
#include "bt_setup.h"
#include "parser.h"
#include "sha1check.h"
#include <Peer.h>
#include <PeerManager.h>

#define BACKLOG 10

using namespace std;

int main (int argc, char * argv[]){

  bt_args_t bt_args;
  int i;

  parse_args(&bt_args, argc, argv);

  if(bt_args.verbose){
    printf("Args:\n");
    printf("verbose: %d\n",bt_args.verbose);
    printf("save_file: %s\n",bt_args.save_file);
    printf("log_file: %s\n",bt_args.log_file);
    printf("torrent_file: %s\n", bt_args.torrent_file);

    for(i=0;i<MAX_CONNECTIONS;i++){
      if(bt_args.peers[i] != NULL)
        print_peer(bt_args.peers[i]);
    }
  }

  //read and parse the torrent file here
  bencodedDict *torrent_args = parse_torrent_file(bt_args.torrent_file);
  bencodedDict *info_dict = (bencodedDict *) torrent_args->getValueForKey("info");
  stringstream info_ss;
  info_dict->printValue(info_ss); 

  int info_str_len = info_ss.str().length();
  char *info_str = new char[info_str_len + 1];
  info_ss.str().copy(info_str, 0, info_str_len);
  info_str[info_str_len] = '\0';

  const char *info_hash = getSHA1(info_str);
  bt_info_t *bt_info = extract_bt_info(torrent_args);

  if(bt_args.verbose){
    // print out the torrent file arguments here
    printf("Torrent file arguments:\n");
    stringstream ss;
    torrent_args->printValue(ss);
    cout << ss.str();
    printf("\n");
  }

  //Setup socket
  struct sockaddr_storage peer_addr;
  socklen_t peer_addr_size;
  struct addrinfo hints, *res;
  int sockfd, new_fd;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  
  printf("Opening port: %s\n", bt_args.port);
  getaddrinfo(NULL, bt_args.port, &hints, &res);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  bind(sockfd, res->ai_addr, res->ai_addrlen);
  listen(sockfd, BACKLOG);

  PeerManager *manager = new PeerManager();
  for (int i = 0; i< MAX_CONNECTIONS; i++) {
    if (bt_args.peers[i] != NULL) {
      Peer *peer = new Peer(bt_args.peers[i], info_hash);
      if (peer->isGood()) {
        printf("Added peer: \n");
        manager->addPeer(peer);
      }
    }
  }
//      threadargs_t *threadargs = (threadargs_t *) malloc(sizeof(threadargs_t));
//      threadargs->peer = bt_args.peers[i];
//      threadargs->bt_info = bt_info;
//      pthread_create(&threads[i], NULL, spawn_thread, (void *) threadargs);
//    }
//  }
  //main client loop
  printf("Starting Main Loop\n");
  while(1){

    //try to accept incoming connection from new peer
    peer_addr_size = sizeof(peer_addr);
    new_fd = accept(sockfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
    Peer *peer = new Peer(new_fd, info_hash);
    if (peer->isGood()) {
      manager->addPeer(peer);
    } 
    
    //poll current peers for incoming traffic
    //   write pieces to files
    //   udpdate peers choke or unchoke status
    //   responses to have/havenots/interested etc.
    
    //for peers that are not choked
    //   request pieaces from outcoming traffic

    //check livelenss of peers and replace dead (or useless) peers
    //with new potentially useful peers
    
    //update peers, 

  }

  return 0;
}
