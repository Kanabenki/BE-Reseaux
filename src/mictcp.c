#include <mictcp.h>
#include <api/mictcp_core.h>
#include <string.h>

#define MAX_SOCKET 10
#define TIMEOUT 10
#define LOSS_RATE 15


enum start_mode current_mode = SERVER;
static struct mic_tcp_sock internal_sock[MAX_SOCKET];
static int curr_socket_nb = 0;
static int expected_seq_num = 0;

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

    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    /* do {
        IP_recv(pdu, addr, TIMEOUT);
    } while (!pdu.header.syn); */
    return 0; //TODO CHANGE
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

    do {
        IP_send(pdu, addr);
        printf("Sent packet seq n \n", pdu.header.seq_num);
        do {
            ip_recv_res = IP_recv(&recv_pdu, &recv_addr, TIMEOUT);
            printf("Received packet\n");
        } while (strcmp(addr.ip_addr, recv_addr.ip_addr) != 0);
        printf("Checked expeditor");
    } while (!(recv_pdu.header.ack == 1 && recv_pdu.header.seq_num == expected_seq_num));

    expected_seq_num = (expected_seq_num+1)%2;
    printf("Received ack seq n %d\n", recv_pdu.header.seq_num);
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
	
	if ((pdu.header.ack == 0) && (pdu.header.syn ==0))
	{
        if (expected_seq_num == pdu.header.seq_num) {
		    app_buffer_put(pdu.payload);
            expected_seq_num = (expected_seq_num + 1) % 2;
        }
		struct mic_tcp_pdu pdu_ack ; 
		pdu_ack.header.ack = 1 ;
		pdu_ack.header.seq_num = pdu.header.seq_num;
        printf("ACK for seq n %d\n", pdu.header.seq_num);
		IP_send(pdu_ack,addr);	
	}
}
