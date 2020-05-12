#include <ncurses.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

unsigned int type;
char name[32];
char mesgBuff[1024], charBuff[1024];
char flagResponse[1];


struct receive_info {
	int sockfd;
	WINDOW* top;
	WINDOW* bottom;
};
// type 1
struct message {
	unsigned short mesgLen;
	char recipientName[32];
	char senderName[32];
	char msg[1024];
} M;

struct changeRoom {
	unsigned short roomNum;
} CR;

struct fight {
} F;

struct pvpFight {
	char fightTargetName[32];
} PVP;

struct loot {
	char lootTargetName[32];
} L;

struct start {
} S;

struct error {
	unsigned short errorCode;
	unsigned short errorMsgLen;
	char errorMsg[1024];
} E;

struct accept {
	unsigned short actionAccepted;
} A;

struct room {
	unsigned short roomNum;
	char roomName[32];
	unsigned short roomDescriptionLen;
	char roomDescription[1024];
} R;

struct character {
	char playerName[32];
	unsigned short flags;
	unsigned short attack;
	unsigned short defense;
	unsigned short regen;
	short health;
	unsigned short gold;
	unsigned short currentRoomNum;
	unsigned short playerDescriptionLength;
	char playerDescription[1024];
} CH;

struct game {
	unsigned short initialPoints;
	unsigned short statLimit;
	unsigned short gamDescriptionLength;
	char gameDescription[1024];
} G;

struct leave {
} LV;

struct connection {
	unsigned short roomNum;
	char roomName[32];
	unsigned short roomDescriptionLen;
	char roomDescription[1024];
} CON;

struct version {
	unsigned short majorRevision;
	unsigned short minorRevision;
	unsigned short extensionsSize;
	char extensionsList[1024];
} V;

//Functions to write to server
void writeCharacter(struct character *CH, struct receive_info *skt){
	wprintw(skt->bottom, "Player name: ");
	wscanw(skt->bottom, "%s", &CH->playerName);
	write(skt->sockfd, &CH->playerName, 32);
	strcpy(name, CH->playerName);

	int joinFlag = 64 & CH->flags;
	wprintw(skt->bottom, "Do you want automatically join battles in the room you are in: ");
	wscanw(skt->bottom, "%s", flagResponse);
	if(flagResponse == "no"){
		joinFlag = 0;
	}
	write(skt->sockfd, &CH->flags, 1);

	wprintw(skt->bottom, "Enter Attack: ");
	wscanw(skt->bottom, "%d", &CH->attack);
	write(skt->sockfd, &CH->attack, 2);

	wprintw(skt->bottom, "Enter Defense: ");
	wscanw(skt->bottom, "%d", &CH->defense);
	write(skt->sockfd, &CH->defense, 2);

	wprintw(skt->bottom, "Enter Regen: ");
	wscanw(skt->bottom, "%d", &CH->regen);
	write(skt->sockfd, &CH->regen, 2);

	wprintw(skt->bottom, "Enter Health: ");
	wscanw(skt->bottom, "%d", &CH->health);
	write(skt->sockfd, &CH->health, 2);

	wprintw(skt->bottom, "Enter Gold: ");
	wscanw(skt->bottom, "%d", &CH->gold);
	write(skt->sockfd, &CH->gold, 2);

	wprintw(skt->bottom, "Enter Room: ");
	wscanw(skt->bottom, "%d", &CH->currentRoomNum);
	write(skt->sockfd, &CH->currentRoomNum, 2);

	wprintw(skt->bottom, "Player Description: ");
	wgetstr(skt->bottom, charBuff);
	strcpy(CH->playerDescription, charBuff);
	CH->playerDescriptionLength = strlen(charBuff);
	
	write(skt->sockfd, &CH->playerDescriptionLength, 2);
	write(skt->sockfd, &CH->playerDescription, CH->playerDescriptionLength);
}

void wrtieMessage(struct message *M, struct receive_info *skt) {
	wprintw(skt->bottom, "Reciptient Name: ");
	wscanw(skt->bottom, "%s", &M->recipientName);

	wprintw(skt->bottom, "Message: ");
	wgetstr(skt->bottom, mesgBuff);
	strcpy(M->msg, mesgBuff);
	M->mesgLen = strlen(mesgBuff);
	strcpy(M->senderName, name);
	
	write(skt->sockfd, &M->mesgLen, 2);
	write(skt->sockfd, &M->recipientName, 32);
	write(skt->sockfd, &M->senderName, 32);
	write(skt->sockfd, &M->msg, M->mesgLen);
}

void writeChangeRoom(struct changeRoom *CR, struct receive_info *skt){
	wprintw(skt->bottom, "Room number: ");
	wscanw(skt->bottom, "%d", &CR->roomNum);
	write(skt->sockfd, &CR->roomNum, 2);
}

void writePVPFight(struct pvpFight *PVP, struct receive_info *skt){
	wprintw(skt->bottom, "Target Name; ");
	wscanw(skt->bottom, "%s", &PVP->fightTargetName);
	write(skt->sockfd, &PVP->fightTargetName, 32);
}

void writeLoot(struct loot *L, struct receive_info *skt){
	wprintw(skt->bottom, "Target Name; ");
	wscanw(skt->bottom, "%s", &L->lootTargetName);
	write(skt->sockfd, &L->lootTargetName, 32);
}

//Functions to read from server
void readMessage(struct message *M, struct receive_info *skt){
	read(skt->sockfd, &M->mesgLen, 2);
	read(skt->sockfd, &M->recipientName, 32);
	read(skt->sockfd, &M->senderName, 32);
	read(skt->sockfd, &M->msg, M->mesgLen);
}

void readError(struct error *E, struct receive_info *skt){
	read(skt->sockfd, &E->errorCode, 1);
	read(skt->sockfd, &E->errorMsgLen, 2);
	read(skt->sockfd, &E->errorMsg, E->errorMsgLen);
}

void readAccept(struct accept *A, struct receive_info *skt){
	read(skt->sockfd, &A->actionAccepted, 1);
}

void readRoom(struct room *R, struct receive_info *skt){
	read(skt->sockfd, &R->roomNum, 2);
	read(skt->sockfd, &R->roomName, 32);
	read(skt->sockfd, &R->roomDescriptionLen, 2);
	read(skt->sockfd, &R->roomDescription, R->roomDescriptionLen);
}

void readCharacter(struct character *CH, struct receive_info *skt){
	read(skt->sockfd, &CH->playerName, 32);
	read(skt->sockfd, &CH->flags, 1);
	read(skt->sockfd, &CH->attack, 2);
	read(skt->sockfd, &CH->defense, 2);
	read(skt->sockfd, &CH->regen, 2);
	read(skt->sockfd, &CH->health, 2);
	read(skt->sockfd, &CH->gold, 2);
	read(skt->sockfd, &CH->currentRoomNum, 2);
	read(skt->sockfd, &CH->playerDescriptionLength, 2);
	read(skt->sockfd, &CH->playerDescription, CH->playerDescriptionLength);
}

void readGame(struct game *G, struct receive_info *skt){
	read(skt->sockfd, &G->initialPoints, 2);
	read(skt->sockfd, &G->statLimit, 2);
	read(skt->sockfd, &G->gamDescriptionLength, 2);
	read(skt->sockfd, &G->gameDescription, G->gamDescriptionLength);
}

void readConnection(struct connection *C, struct receive_info *skt){
	read(skt->sockfd, &C->roomNum, 2);
	read(skt->sockfd, &C->roomName, 32);
	read(skt->sockfd, &C->roomDescriptionLen, 2);
	read(skt->sockfd, &C->roomDescription, C->roomDescriptionLen);
}

void readVersion(struct version *V, struct receive_info *skt){
	read(skt->sockfd, &V->majorRevision, 1);
	read(skt->sockfd, &V->minorRevision, 1);
	read(skt->sockfd, &V->extensionsSize, 2);
	read(skt->sockfd, &V->extensionsList, V->extensionsSize);
}


void exit_graceful(int signal){
	endwin();
	exit(0);
}

void* receive_print(void* arg){
	struct receive_info* prfi = (struct receive_info*)arg;
	char readstring[1024*1024];
	ssize_t readsize;
	for(;;){
		read(prfi->sockfd, &type, 1);

		if(type == 14){
			readVersion(&V, prfi);
			wrefresh(prfi->top);
			wprintw(prfi->top, "Type: %d \n", type);
			wprintw(prfi->top, "LURK Version: %d.%d \n", V.majorRevision, V.minorRevision);
			wrefresh(prfi->top);
		}
		else if (type == 11){
			readGame(&G, prfi);
			wrefresh(prfi->top);
			wprintw(prfi->top, "Type: %d \n", type);
			wprintw(prfi->top, "Initial Points: %d \n", G.initialPoints);
			wprintw(prfi->top, "Stat Limit: %d \n", G.statLimit);
			wprintw(prfi->top, "Game Description: %s \n", G.gameDescription);
			wrefresh(prfi->top);
			memset(G.gameDescription, 0, sizeof(G.gameDescription));
		}
		else if(type == 13) {
			readConnection(&CON, prfi);
			wrefresh(prfi->top);
			wprintw(prfi->top, "Type: %d \n", type);
			wprintw(prfi->top, "Room Number: %d \n", CON.roomNum);
			wprintw(prfi->top, "Room Name: %s \n", CON.roomName);
			wprintw(prfi->top, "Description: %s \n", CON.roomDescription);
			wrefresh(prfi->top);
			memset(CON.roomName, 0, sizeof(CON.roomName));
			memset(CON.roomDescription, 0, sizeof(CON.roomDescription));
		}

		else if(type == 10){
			readCharacter(&CH, prfi);
			wrefresh(prfi->top);
			int alive = 128 & CH.flags;
			int joinBattle = 64 & CH.flags;
			int monster = 32 & CH.flags;
			int started = 16 & CH.flags;
			int ready = 8 & CH.flags;
			wprintw(prfi->top, "Type: %d \n", type);
			wprintw(prfi->top, "Player name: %s \n", CH.playerName);
			wprintw(prfi->top, "Flags: %d \n", CH.flags);
			wprintw(prfi->top, "Attack: %d \n", CH.attack);
			wprintw(prfi->top, "Defense: %d \n", CH.defense);
			wprintw(prfi->top, "Health: %d \n", CH.health);
			wprintw(prfi->top, "Character Description: %s \n", CH.playerDescription);
			if(!alive){
				wprintw(prfi->top, "Status: Dead \n");
			}
			else{
				wprintw(prfi->top, "Status: Alive \n");
			}
			if(!monster){
				wprintw(prfi->top, "Current room %d \n", CH.currentRoomNum);
				wprintw(prfi->top, "Gold: %d \n", CH.gold);
			}
			if(!started){
				wprintw(prfi->top, "Status: Not Started \n");
			}
			else {
				wprintw(prfi->top, "Status: Started \n");
			}
			if(!joinBattle){
				wprintw(prfi->top, "Status: Battles not joined \n");
			}
			else {
				wprintw(prfi->top, "Status: Battles joined \n");
			}
			if(!ready){
				wprintw(prfi->top, "Status: Not Ready \n");
			}
			else {
				wprintw(prfi->top, "Status: Ready \n");
			}
			wrefresh(prfi->top);
			memset(CH.playerName, 0, sizeof(CH.playerName));
			memset(CH.playerDescription, 0, CH.playerDescriptionLength);

		}
		else if(type == 9){
			readRoom(&R, prfi);
			wrefresh(prfi->top);
			wprintw(prfi->top, "Type: %d \n", type);
			wprintw(prfi->top, "Current room number: %d \n", R.roomNum);
			wprintw(prfi->top, "Room name: %s \n", R.roomName);
			wprintw(prfi->top, "Room Description: %s \n", R.roomDescription);
			wrefresh(prfi->top);
			memset(R.roomName, 0, sizeof(R.roomName));
			memset(R.roomDescription, 0, R.roomDescriptionLen);
		}
		else if(type == 8){
			wrefresh(prfi->top);
			readAccept(&A, prfi);
			wprintw(prfi->top, "Type: %d \n", type);
			wprintw(prfi->top, "Accepted: %d \n", A.actionAccepted);
			wrefresh(prfi->top);
		}
		else if(type == 7){
			wrefresh(prfi->top);
			readError(&E, prfi);
			wprintw(prfi->top, "Type: %d \n", type);
			wprintw(prfi->top, "Error Code: %d \n", E.errorCode);
			wprintw(prfi->top, "Error Message: %s \n", E.errorMsg);
			wrefresh(prfi->top);
			memset(E.errorMsg, 0, E.errorMsgLen); //Dont know if this is going to work
		}
		else if(type == 1){
			wrefresh(prfi->top);
			readMessage(&M, prfi);
			wprintw(prfi->top, "Type: %d \n", type);
			wprintw(prfi->top, "Recipient Name: %s \n", M.recipientName);
			wprintw(prfi->top, "Sender Name: %s \n", M.senderName);
			wprintw(prfi->top, "Message: %s \n", M.msg);
			wrefresh(prfi->top);
			memset(M.senderName, 0, sizeof(M.senderName));
			memset(M.recipientName, 0, sizeof(M.recipientName));
			memset(M.msg, 0, M.mesgLen);
		}


		wprintw(prfi->top, "\n");
		wmove(prfi->bottom, LINES/4 - 2, 0);
		wrefresh(prfi->top);
		wrefresh(prfi->bottom);
	}
	return 0; 

}

int main(int argc, char ** argv){
	// Usage Information
	if(argc < 3){
		printf("Usage:  %s hostname port\n", argv[0]);
		return 1;
	}

	// Handle Signals
	struct sigaction sa;
	sa.sa_handler = exit_graceful;
	sigaction(SIGINT, &sa, 0);

	// Prepare the network connection, but don't call connect yet
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
		goto err;
	short port = atoi(argv[2]);
	struct sockaddr_in connect_addr;
	connect_addr.sin_port = htons(port);
	connect_addr.sin_family = AF_INET;
	struct hostent* entry = gethostbyname(argv[1]);
	if(!entry)
		goto err;
	struct in_addr **addr_list = (struct in_addr**)entry->h_addr_list;
	struct in_addr* c_addr = addr_list[0];
	char* ip_string = inet_ntoa(*c_addr);
	connect_addr.sin_addr = *c_addr;
	
	// Set up curses.  This is probably easier in Python.
	initscr();
	start_color();
	use_default_colors();
	init_pair(1, COLOR_GREEN, -1);
	init_pair(2, COLOR_RED, -1);
	refresh();
	WINDOW* top = newwin(LINES*3/4, COLS, 0, 0);
	WINDOW* bottom = newwin(LINES/4 - 1, COLS, LINES*3/4 + 1, 0);
	refresh();
	wmove(stdscr, LINES*3/4, 0);
	whline(stdscr, ACS_HLINE , COLS);
	wmove(bottom, LINES/4 - 2, 0);
	scrollok(bottom, 1);
	scrollok(top, 1);
	wrefresh(top);
	wrefresh(bottom);
	refresh();

	// The UI is up, let's reassure the user that whatever name they typed resolved to something
	wattron(top, COLOR_PAIR(1));
	wprintw(top, "Connecting to host %s (%s)\n", entry->h_name, ip_string);
	wrefresh(top);
	
	// Actually connect.  It might connect right away, or sit here and hang - depends on how the
	// host is feeling today
	if(connect(sockfd, (struct sockaddr*)&connect_addr, sizeof(struct sockaddr_in)))
		goto err;
	
	// Let the user know we're connected, so they can start doing whatever they do.
	wprintw(top, "Connected\n");
	wrefresh(top);
	wattroff(top, COLOR_PAIR(1));

	// Start the receive thread
	struct receive_info rfi;
	rfi.sockfd = sockfd;
	rfi.top = top;
	rfi.bottom = bottom;
	pthread_t t;
	pthread_create(&t, 0, receive_print, &rfi);
	
	// Get user input.  Ctrl + C is the way out now.
	char input[1024*1024];
	wmove(bottom, LINES/4 - 2, 0);
	wgetnstr(bottom, input, 1024*1024-1); 
	// wprintw(bottom, "For a list of game commands type help\n");
	for(;;){
	
		wscrl(bottom, 0);
		wrefresh(bottom);
		wgetnstr(bottom, input, 1024*1024-1); 
		if((strcmp(input, "Create Character")) == 0){
			type = 10;
			write(sockfd, &type, 1);
			writeCharacter(&CH, &rfi);	
		}
		else if((strcmp(input, "Leave Game")) == 0){
			type = 11;
			write(sockfd, &type, 1);
			endwin();
			exit(0);
		}
		else if((strcmp(input, "Start")) == 0){
			type = 6;
			write(sockfd, &type, 1);
		}
		else if((strcmp(input, "Loot Player")) == 0){
			type = 5;
			write(sockfd, &type, 1);
			writeLoot(&L, &rfi);
		}
		else if((strcmp(input, "PVP Fight")) == 0){
			type = 4;
			write(sockfd, &type, 1);
			writePVPFight(&PVP, &rfi);
		}
		else if((strcmp(input, "Fight")) == 0){
			type = 3;
			write(sockfd, &type, 1);
		}
		else if((strcmp(input, "Change Room")) == 0){
			type = 2;
			write(sockfd, &type, 1);
			writeChangeRoom(&CR, &rfi);
		}
		else if((strcmp(input, "Message")) == 0){
			type = 1;
			write(sockfd, &type, 1);
			wrtieMessage(&M, &rfi);
		}
		else if((strcmp(input, "Clear Screen")) == 0){
			wclear(top);
		}
		else if((strcmp(input, "help")) == 0) {
			wprintw(bottom, "Create Character: creates a new character \n");
			wprintw(bottom, "Leave Game: leaves the game \n");
			wprintw(bottom, "Start: starts the game \n");
			wprintw(bottom, "Loot Player: takes gold from dead player \n");
			wprintw(bottom, "PVP Fight: lets you fight a specific person \n");
			wprintw(bottom, "Fight: starts a fight with the monsters in room \n");
			wprintw(bottom, "Change Room: changes the room you are in \n");
			wprintw(bottom, "Message: sends a message to another player \n");
		}
		else{
			wprintw(bottom, "Please enter a valid option! \n");
			wprintw(bottom, "For a list of game commands type help\n");

		}
	}

err:
	endwin();
	perror(argv[0]);
	return 1;
}












































