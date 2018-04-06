#include <mictcp.h>
#include <api/mictcp_core.h>
#include <string.h>

#define MAX_SOCKET 10
#define TIMEOUT 1000
#define LOSS_ARR_SIZE 100
#define LOSS_RATE 50
#define MAX_LOSS_RATE 30


enum start_mode current_mode = SERVER;
static struct mic_tcp_sock internal_sock[MAX_SOCKET];
static int curr_socket_nb = 0;
static int expected_seq_num = 0;
static int total_pack_sent = 0;
// 1 loss 0 received
static int loss_status[MAX_SOCKET][LOSS_ARR_SIZE];

int current_loss_rate(int socket){
int lost = 0 ;
	for(int i =0; i<LOSS_ARR_SIZE;i++){
		lost+=loss_status[socket][i];
	}
	lost = ((float) lost/ (float) LOSS_ARR_SIZE)*100;	 // returns valeur en pourcentage 0 < lost <100
	return lost ;
}

/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm)
{
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   if (initialize_components(sm) == -1) {
       return -1;
   } /* Appel obligatoire */
    set_loss_rate(LOSS_RATE);
    current_mode = sm;
    if (curr_socket_nb >= MAX_SOCKET) {
        return -1;
    }
    internal_sock[curr_socket_nb].fd = curr_socket_nb;
    memset(&loss_status[curr_socket_nb], 0, sizeof(loss_status[curr_socket_nb]));
    curr_socket_nb++;
    return curr_socket_nb-1;
}

/*
 * Permet d’attribuer une adresse à un socket.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr)
{
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   if (socket < 0 || socket >= curr_socket_nb) {
       return -1;
   }
   internal_sock[socket].addr = addr;
   return 0;
}

/*
 * Met le socket en état d'acceptation de connexions
 * Retourne 0 si succès, -1 si erreur
 */
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr)
{
    struct mic_tcp_pdu pdu;
    struct mic_tcp_sock_addr;
	int istimeout = 0 ;

    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
/*      do {
        istimeout = IP_recv(pdu, addr, TIMEOUT);
    } while ((!pdu.header.syn) && (istimeout != -1)); 
	
	if (istimeout != -1) {
			printf("reçu un SYN\n");
			struct mic_tcp_pdu pdu_syn_ack ; 
			pdu_ack.header.ack = 1 ;
			pdu_syn_ack_.header.syn = 1;
			printf("Envoi SYN_ACK\n");
			IP_send(pdu_ack,addr);	
		}
		do {
			istimeout = IP_rev(pdu,addr,TIMEOUT);
		} while ((!pdu.header.ack) && (istimeout != -1));
	
		if (istimeout!=-1)	
			printf("reçu ACK\n"); 
    		return 0;
		} else {
			return -1 ;
		} 

	} else {
		return -1;
	} */
    return 0;
}

/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */
int mic_tcp_connect(int socket, mic_tcp_sock_addr addr)
{
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    return 0; //TODO CHANGE
}

/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */

int mic_tcp_send (int socket, char* mesg, int mesg_size)
{
    int ip_recv_res;
    struct mic_tcp_pdu pdu, recv_pdu;
    struct mic_tcp_sock_addr addr, recv_addr;
    addr.ip_addr = "localhost";
    addr.ip_addr_size = strlen("localhost");
    addr.port = 10000;

    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");

    if (socket < 0 || socket >= curr_socket_nb) {
        return -1;
    }

    memset(&pdu, 0, sizeof(pdu));
    pdu.header.source_port = 10000;
    pdu.header.dest_port = 10000; //TODO change

    pdu.header.seq_num = expected_seq_num;

    pdu.payload.data = mesg;
    pdu.payload.size = mesg_size;
	

/*     do {
        IP_send(pdu, addr);
        tries++;
        printf("Sent packet seq n \n", pdu.header.seq_num);
        ip_recv_res = IP_recv(&recv_pdu, &recv_addr, TIMEOUT);
        if (!(ip_recv_res >= 0 && recv_pdu.header.ack == 1 && recv_pdu.header.seq_num == expected_seq_num)
            && tries > 1 && current_loss_rate(socket) < MAX_LOSS_RATE) {
            loss_status[socket][expected_seq_num % LOSS_ARR_SIZE] = 1;
            break;
        }
    } while (!(ip_recv_res >= 0 && recv_pdu.header.ack == 1 && recv_pdu.header.seq_num == expected_seq_num));
 */
    int is_lost = 0;
    printf("[MIC-TCP-SEND] LOSS RATE BEF SEND: %d\n", current_loss_rate(socket));
    IP_send(pdu, addr);
    printf("[MIC-TCP-SEND] SENT N %d \n", pdu.header.seq_num);
    ip_recv_res = IP_recv(&recv_pdu, &recv_addr, TIMEOUT);
    while (!(ip_recv_res >= 0 && recv_pdu.header.ack == 1 && recv_pdu.header.seq_num == expected_seq_num)) {
        printf("[MIC-TCP-RECV] PACK recu: res: %d - ack: %d - recv_seq_num: %d - exp_seq_num: %d\n", ip_recv_res, recv_pdu.header.ack, recv_pdu.header.seq_num, expected_seq_num);
        if (current_loss_rate(socket) < MAX_LOSS_RATE) {
            printf("[MIC-TCP-SEND] SKIPPING N %d \n", pdu.header.seq_num);
            is_lost = 1;
            break;
        }
        IP_send(pdu, addr);
        printf("[MIC-TCP-SEND] SENT N %d \n", pdu.header.seq_num);
        ip_recv_res = IP_recv(&recv_pdu, &recv_addr, TIMEOUT);
    }

    loss_status[socket][total_pack_sent % LOSS_ARR_SIZE] = is_lost;
    printf("[MIC-TCP-SEND] LOSS RATE AFT SEND: %d\n", current_loss_rate(socket));
    
    total_pack_sent++;
    if (!is_lost) {
        expected_seq_num++;
        printf("[MIC-TCP-SEND] RECEV ACK N %d\n", recv_pdu.header.seq_num);
    }
    
    return 0;
}

/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
 * NB : cette fonction fait appel à la fonction app_buffer_get()
 */
int mic_tcp_recv (int socket, char* mesg, int max_mesg_size)
{
    struct mic_tcp_payload payload;
	
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    if (socket < 0 || socket >= curr_socket_nb) {
        return -1;
    }
	 
    payload.data = mesg;
    payload.size = max_mesg_size;
    return app_buffer_get(payload);
}

/*
    IP_recv(&pdu, &addr, TIMEOUT);
    IP_recv(&pdu, &addr, TIMEOUT);
 * Permet de réclamer la destruction d’un socket.
 * Engendre la fermeture de la connexion suivant le modèle de TCP.
 * Retourne 0 si tout se passe bien et -1 en cas d'erreur
 */
int mic_tcp_close (int socket)
{
    printf("[MIC-TCP] Appel de la fonction :  "); printf(__FUNCTION__); printf("\n");
    return 0;
}

/*
 * Traitement d’un PDU MIC-TCP reçu (mise à jour des numéros de séquence
 * et d'acquittement, etc.) puis insère les données utiles du PDU dans
 * le buffer de réception du socket. Cette fonction utilise la fonction
 * app_buffer_put().
 */
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_sock_addr addr){

    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    printf("[MIC-TCP-PROCESS-PDU] EXPECTING N %d\n", expected_seq_num);
	if ((pdu.header.ack == 0) && (pdu.header.syn ==0)) 
    {
        printf("[MIC-TCP-PROCESS-PDU] RECEIVED N %d\n", pdu.header.seq_num);
        if (expected_seq_num == pdu.header.seq_num) {
		    app_buffer_put(pdu.payload);
            expected_seq_num++;
        }
		struct mic_tcp_pdu pdu_ack;
		pdu_ack.header.ack = 1;
		pdu_ack.header.seq_num = pdu.header.seq_num;
        printf("[MIC-TCP-RECEV] RECEV N %d\n", pdu.header.seq_num);
		IP_send(pdu_ack,addr);	
	}
}
