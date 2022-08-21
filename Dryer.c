#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
 
int main() {

	char pathname[] = "WasherProcess.txt";
	key_t key;
	
	struct mymsgbuf {
		long mtype;
		int DishesType;
		int FreePlace;
	}mybuf;
	
	if((key = ftok(pathname, 0)) < 0) {
		printf("Невозможно сгенерировать ключ\n");
		exit(-1);
	}
	int msqid;
	
	if((msqid = msgget(key, 0666 | IPC_CREAT)) < 0) {
		printf("Невозможно получить IPC дескриптор для очереди сообщений\n");
		exit(-2);
	}
	
	int length = sizeof(struct mymsgbuf);
	
	FILE *DryerProcess;

	if ((DryerProcess = fopen("DryerProcess.txt", "r")) == NULL) {
		printf("Невозможно открыть файл DryerProcess.txt\n");
		exit(-1);
	}
	
	char OnlineDryer[5];
	int TimeDryer, type;
		
	while(1) {
		if ((msgrcv(msqid, (struct mymsgbuf*) &mybuf, length, 1, 0)) < 0) {
			printf("Невозможно получить сообщение из очереди\n");
			fclose(DryerProcess);
			exit(4);
		}

		if(mybuf.DishesType == -1) {
			msgctl(msqid, IPC_RMID, (struct msqid_ds*) NULL);
			exit(0);
		}
		
		rewind(DryerProcess);
		
		do {
			fgets(OnlineDryer, sizeof(OnlineDryer), DryerProcess);
			sscanf(OnlineDryer, "%d:%d\n", &type, &TimeDryer);
		} while (type != mybuf.DishesType);
		printf("Вытираю\n");
		sleep(TimeDryer);
		puts("Закончил вытирать");
		mybuf.FreePlace = mybuf.FreePlace + 1;
		mybuf.mtype = 2;
		
		if (msgsnd(msqid, (struct mymsgbuf*) &mybuf, length, 0) < 0) {

			printf("Невозможно отправить сообщение\n");
			msgctl(msqid, IPC_RMID, (struct msqid_ds*) NULL);
			fclose(DryerProcess);
			exit(3);
		}
	}
	
	return 0;
}
