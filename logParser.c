#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#define file argv[1]
#define REQUEST_LEN 2048
#define STR_MAX_LEN 4096

char *month[] = {"Jan", "Feb", "Mar", "Apr",
                 "May", "Jun", "Jul", "Aug",
                 "Sep", "Oct", "Nov", "Dec"};

short getMonth(char *str) {
    int8_t ans = 0;
    for (int8_t i = 0; i < 12; i++) {
        if (strcmp(str, month[i]) == 0) {
            ans = i;
            break;
        }
    }
    return ans;
}

struct Log {
    char request[REQUEST_LEN];
    bool status;
    bool success;
} typedef Log;


struct Node {
    time_t time;
    struct Node *next;
} typedef Node;


struct MyQueue {
    size_t size;
    Node *first;
    Node *last;
} typedef MyQueue;


void deleteFirst(MyQueue *queue) {
    if (queue->size == 0) {
        return;
    }
    if (queue->size == 1) {
        free(queue->first);
        queue->first = NULL;
        queue->last = NULL;
    } else {
        Node *tmp = queue->first;
        queue->first = tmp->next;
        free(tmp);
    }
    queue->size--;
}


void addNew(MyQueue *queue, time_t time) {
    Node *tmp = malloc(sizeof(Node));
    tmp->time = time;
    if (queue->size == 0) {
        tmp->next = NULL;
        queue->first = queue->last = tmp;
    } else {
        tmp->next = NULL;
        queue->last->next = tmp;
        queue->last = tmp;
    }
    queue->size++;
}


void strParsing(char *str, time_t *timeRes, char *request, bool *status, bool *success) {
    *status = false;
    *success = false;
    int ind = 0;
    char mon[4];
    struct tm time;

    if (sscanf(str, "%*s - - [%d/%3s/%d:%d:%d:%d",
               &time.tm_mday, mon, &time.tm_year, &time.tm_hour, &time.tm_min, &time.tm_sec) == 6) {
        time.tm_mon = getMonth(mon);
        time.tm_year -= 1900;
        *timeRes = mktime(&time);
    } else {
        *success = false;
    }


    while (str[ind] != '\"') {
        ind++;
    }

    request[0] = str[ind++];
    int index = 1;
    while (str[ind] != '\"') {
        request[index++] = str[ind++];
    }
    request[index++] = '\"';
    request[index] = '\0';


    if (str[ind + 2] == '5') {
        *status = true;
    }
}

void readFile(FILE *in, int period) {
    size_t number_of_bad_requests = 0;
    int ind = 0;

    size_t maxSize = 0;
    time_t lowestBound = 0;
    struct MyQueue queue;
    queue.size = 0;
    queue.last = NULL;
    queue.first = NULL;
    char str[STR_MAX_LEN];
    Log cur;

    while (!feof(in)) {
        ind++;
        fgets(str, STR_MAX_LEN, in);
        time_t currentTime;
        strParsing(str, &currentTime, cur.request, &cur.status, &cur.success);
        if (cur.status) {
            printf("%s\n", cur.request);
            number_of_bad_requests++;
        }
        if (queue.size == 0) {
            lowestBound = currentTime;
        }
        if (currentTime - lowestBound <= period) { ;
            addNew(&queue, currentTime);
        } else {
            while (queue.size != 0 && currentTime - lowestBound > period) {
                deleteFirst(&queue);
                if (queue.size != 0) {
                    lowestBound = queue.first->time;
                }
            }
            addNew(&queue, currentTime);
            lowestBound = queue.first->time;
        }
        if (queue.size >= maxSize) {
            maxSize = queue.size;
        }
    }

    while (queue.size != 0) {
        deleteFirst(&queue);
    }
    printf("number = %d\n", number_of_bad_requests);
    printf("Max number of requests in [t, t + %i]: %zu\n", period, maxSize);
}

int main(int argc, char **argv) {
    FILE *in = fopen(file, "r");

    if (argc != 2){
        printf("%s", "Usage of program: InputFile");
    }

    if (in == NULL) {
        printf("%s", "InputFile open error");
        fclose(in);
        return -1;
    }
    int period;

    printf("%s", "Enter the period of time: ");
    scanf("%i", &period);


    readFile(in, period);
    fclose(in);
}