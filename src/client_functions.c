#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "utils.h"
#include "client_functions.h"
#include "logger/logger.h"

char login_response[512];
char get_products_response[5096]; // ~250 products can be returned in a single response
char get_customers_response[5096]; // ~150 customers can be returned in a single response
char get_orders_response[5096]; // ~250 orders can be returned in a single response
char get_customers_combobox_response[5096]; // ~500 customers can be returned in a single response
char get_products_combobox_response[5096]; // ~500 customers can be returned in a single response
char get_users_response[5096]; // ~100 users can be returned in a single response


/* -------------- CLIENT-SERVER INTERACTIONS -------------- */

int connect_to_server(){
    set_log_file("logs/client.log");

    struct sockaddr_in serv_addr;
    int sock;

    printf("Creating client socket...\n");
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    printf("Defining client socket family: (address, port)...\n");
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_ADDRESS, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }

    printf("Client is connecting to the server...\n");
    while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed. Retrying...");
        sleep(1);
        printf("Cennection failed. Retrying...");
    }

    printf("Connection established.\n");
    return sock;
}

int send_to_server(int sock, const char *data) {
    set_log_file("logs/client.log");

    size_t data_len = strlen(data);
    if (send(sock, data, data_len, 0) != data_len) {
        fprintf(stderr, "Error sending data.\n");
        disconnect_from_server(sock);
        return 1;
    }
    printf("Data sent successfully. The data: %s\n", data);
    return 0;
}

int disconnect_from_server(int sock) {
    // Close the socket
    close(sock);
    printf("Disconnected from the server.\n");
    return 0;
}



/* -------------- AUTH FUNCTIONS -------------- */

int register_user(int sock_fd, const char *username, const char *password) {
    set_log_file("logs/client.log");

    char message[512];
    snprintf(message, sizeof(message), "REGISTER|%s|%s", username, password);

    if (send(sock_fd, message, strlen(message), 0) == -1) {
        perror("Failed to send register request");
        return 1;
    }

    printf("DEBUG::: Register/Register button clicked, data sent.");

    char response[512];
    int bytes_received = recv(sock_fd, response, sizeof(response) - 1, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive register response");
        return 2;
    }
    response[bytes_received] = '\0';

    if (strncmp(response, "register_success", 16) == 0) {
        printf("Registration successful! Assigned customerID: %s\n", response + 17);
        return 0;
    }
    printf("Registration failed: %s\n", response);
    return -1;
}


char* login_user(int sock_fd, const char *username, const char *password) {
    set_log_file("logs/client.log");

    char message[512];
    snprintf(message, sizeof(message), "LOGIN|%s|%s", username, password);
    logger("DEBUG", "Sending login request: %s", message);

    if (send(sock_fd, message, strlen(message), 0) == -1) {
        perror("Failed to send login request");
        return "1";
    }

    logger("INFO", "Sent login request: %s", message);

    // char response[512];
    int bytes_received = recv(sock_fd, login_response, sizeof(login_response) - 1, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive login response");
        logger("ERROR", "Failed to receive login response");
        return "2";
    }
    login_response[bytes_received] = '\0';
    logger("INFO", "Received login response: %s", login_response);

    if (strncmp(login_response, "true", 4) == 0) {
        // printf("Login successful! Assigned customerID: %s\n", login_response + 5);
        // printf("%s\n",login_response);
        logger("INFO", "Login successful! Assigned customerID: %s\n", login_response + 5);
        return login_response;
    }
    printf("Login failed: Invalid username or password.\n");
    return "-1";
}



/* -------------- PRODUCTS FUNCTIONS -------------- */

char* get_products(int sock_fd) {
    char message[512];
    snprintf(message, sizeof(message), "GET_PRODUCTS");

    if (send(sock_fd, message, strlen(message), 0) == -1) {
        perror("Failed to send get_products request");
        return "1";
    }

    int bytes_received = recv(sock_fd, get_products_response, sizeof(get_products_response) - 1, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive get_products response");
        return "2";
    }
    get_products_response[bytes_received] = '\0';

    if (strncmp(get_products_response, "true", 4) == 0) {
        printf("Get products successful!");
        return get_products_response;
    }
    printf("Get products failed.\n");
    return "-1";
}

int add_product(int sock_fd, const char *name, const int price) {
    char message[512];
    snprintf(message, sizeof(message), "CREATE_PRODUCT|%s|%d", name, price);

    if (send(sock_fd, message, strlen(message), 0) == -1) {
        perror("Failed to send add_product request");
        return 1;
    }

    return 0;
}

int update_product(int sock_fd, int id, const char *name, const int price) {
    char message[512];
    snprintf(message, sizeof(message), "UPDATE_PRODUCT|%d|%s|%d", id, name, price);

    if (send(sock_fd, message, strlen(message), 0) == -1) {
        perror("Failed to send add_product request");
        printf("Update product request failed.\n");
        return 1;
    }

    return 0;
}

int delete_product(int sock_fd, int id) {
    char message[512];
    snprintf(message, sizeof(message), "DELETE_PRODUCT|%d", id);

    if (send(sock_fd, message, strlen(message), 0) == -1) {
        perror("Failed to send delete_product request");
        printf("Delete product request failed.\n");
        return 1;
    }

    return 0;
}



/* -------------- CUSTOMERS FUNCTIONS -------------- */

char* get_customers(int sock_fd) {
    char message[512];
    snprintf(message, sizeof(message), "GET_CUSTOMERS");

    if (send(sock_fd, message, strlen(message), 0) == -1) {
        perror("Failed to send get_customers request");
        return "1";
    }

    int bytes_received = recv(sock_fd, get_customers_response, sizeof(get_customers_response) - 1, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive get_customers response");
        return "2";
    }
    get_customers_response[bytes_received] = '\0';

    if (strncmp(get_customers_response, "true", 4) == 0) {
        printf("Get customers successful!");
        return get_customers_response;
    }
    printf("Get customers failed.\n");
    return "-1";
}


int add_customer(int sock_fd, const char *fname, const char *lname, const char *phone_number){
    char message[512];
    snprintf(message, sizeof(message), "CREATE_CUSTOMER|%s|%s|%s", fname, lname, phone_number);

    if (send(sock_fd, message, strlen(message), 0) == -1) {
        perror("Failed to send add_customer request");
        return 1;
    }

    return 0;
}


int update_customer(int sock_fd, int id, const char *fname, const char *lname, const char *phone_number){
    char message[512];
    snprintf(message, sizeof(message), "UPDATE_CUSTOMER|%d|%s|%s|%s", id, fname, lname, phone_number);

    if (send(sock_fd, message, strlen(message), 0) == -1) {
        perror("Failed to send update_customer request");
        printf("Update customer request failed.\n");
        return 1;
    }

    return 0;
}

int delete_customer(int sock_fd, int id) {
    char message[512];
    snprintf(message, sizeof(message), "DELETE_CUSTOMER|%d", id);

    if (send(sock_fd, message, strlen(message), 0) == -1) {
        perror("Failed to send delete_customer request");
        printf("Delete customer request failed.\n");
        return 1;
    }

    return 0;
}



/* -------------- ORDERS FUNCTIONS -------------- */

char* get_orders(int sock_fd) {
    char message[512];
    snprintf(message, sizeof(message), "GET_ORDERS");

    if (send(sock_fd, message, strlen(message), 0) == -1) {
        perror("Failed to send get_orders request");
        return "1";
    }

    int bytes_received = recv(sock_fd, get_orders_response, sizeof(get_orders_response) - 1, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive get_orders_response");
        return "2";
    }
    get_orders_response[bytes_received] = '\0';

    if (strncmp(get_orders_response, "true", 4) == 0) {
        printf("Get orders successful!");
        return get_orders_response;
    }
    printf("Get orders failed.\n");
    return "-1";
}


char* get_customers_combobox(int sock_fd){
    char message[512];
    snprintf(message, sizeof(message), "GET_CUSTOMERS_COMBOBOX");

    if (send(sock_fd, message, strlen(message), 0) == -1) {
        perror("Failed to send get_customers_combobox request");
        return "1";
    }

    int bytes_received = recv(sock_fd, get_customers_combobox_response, sizeof(get_customers_combobox_response) - 1, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive get_customers_combobox response");
        return "2";
    }
    get_customers_combobox_response[bytes_received] = '\0';

    if (strncmp(get_customers_combobox_response, "true", 4) == 0) {
        printf("Get customers combobox successful!\n");
        return get_customers_combobox_response;
    }
    printf("Get customers combobox failed.\n");
    return "-1";
}

char* get_products_combobox(int sock_fd){
    char message[512];
    snprintf(message, sizeof(message), "GET_PRODUCTS_COMBOBOX");

    if (send(sock_fd, message, strlen(message), 0) == -1) {
        perror("Failed to send get_products_combobox request");
        return "1";
    }

    int bytes_received = recv(sock_fd, get_products_combobox_response, sizeof(get_products_combobox_response) - 1, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive get_products_combobox response");
        return "2";
    }
    get_products_combobox_response[bytes_received] = '\0';

    if (strncmp(get_products_combobox_response, "true", 4) == 0) {
        printf("Get products combobox successful: %s\n", get_products_combobox_response);
        return get_products_combobox_response;
    }
    printf("Get products combobox failed.\n");
    return "-1";
}

/* -------------- USERS FUNCTIONS -------------- */

char* get_users(int sock_fd) {
    char message[512];
    snprintf(message, sizeof(message), "GET_USERS");

    if (send(sock_fd, message, strlen(message), 0) == -1) {
        perror("Failed to send get_users request");
        return "1";
    }

    int bytes_received = recv(sock_fd, get_users_response, sizeof(get_users_response) - 1, 0);
    if (bytes_received <= 0) {
        perror("Failed to receive get_users response");
        return "2";
    }
    get_users_response[bytes_received] = '\0';

    if (strncmp(get_users_response, "true", 4) == 0) {
        printf("Get users successful!");
        return get_users_response;
    }
    printf("Get users failed.\n");
    return "-1";
}