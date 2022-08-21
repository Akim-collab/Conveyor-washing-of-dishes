#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define TABLE_LIMIT 2
 
int main() {	
	
	int Dishes = open("DirtyDishes.txt", O_RDONLY | O_CLOEXEC);
	if(Dishes == -1) {
		printf("Невозможно работать с файлом DirtyDishes.txt\n");
		close(Dishes);
		exit(1);
	}
	close(Dishes);
	
	int Washer = open("WasherProcess.txt", O_RDONLY | O_CLOEXEC);
	if(Washer == -1) {
		printf("Невозможно работать с файлом WasherProcess.txt\n");
		close(Washer);
		exit(1);
	}
	close(Washer);
	
	int Dryer = open("DryerProcess.txt", O_RDONLY | O_CLOEXEC);
	if(Dryer == -1) {
		printf("Невозможно работать с файлом DryerProcess.txt\n");
		close(Dryer);
		exit(1);
	}
	close(Dryer);

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
	
	mybuf.FreePlace = TABLE_LIMIT;
	
	int len = sizeof(struct mymsgbuf);
	
	FILE *DirtyDishes;
	FILE *WasherProcess;
	
	if((DirtyDishes = fopen("DirtyDishes.txt", "r")) == NULL) {
		printf("Невозможно открыть файл DirtyDishes.txt\n");
		exit(-1);
	}
	
	if((WasherProcess = fopen("WasherProcess.txt", "r")) == NULL) { 
		printf("Невозможно открыть файл WasherProcess.txt\n");
		exit(-1);
	}

	char OnlineDirtyDishes[5], OnlineWasher[5];
	
	int QuantityDirtyDishes, TimeWasher, type;

	while(fgets(OnlineDirtyDishes, sizeof(OnlineDirtyDishes), DirtyDishes) != NULL) {
		
		sscanf(OnlineDirtyDishes, "%d:%d\n", &mybuf.DishesType, &QuantityDirtyDishes);
		printf("Тип посуды - %d, количество - %d\n", mybuf.DishesType, QuantityDirtyDishes);
		rewind(WasherProcess);

		do {
			fgets(OnlineWasher, sizeof(OnlineWasher), WasherProcess);
			sscanf(OnlineWasher, "%d:%d\n", &type, &TimeWasher);
		} while(type != mybuf.DishesType);
		
		while(QuantityDirtyDishes != 0) {
			puts("Мою\n");
			sleep(TimeWasher);
			puts("Закончил мыть\n");
			mybuf.FreePlace = mybuf.FreePlace - 1;
			QuantityDirtyDishes = QuantityDirtyDishes - 1;
			mybuf.mtype = 1;
			
			if (msgsnd(msqid, (struct msgbuf*) &mybuf, len, 0) < 0) {
				printf("Невозможно отправить сообщение\n");
				fclose(DirtyDishes);
        			fclose(WasherProcess);
				msgctl(msqid, IPC_RMID, (struct msqid_ds*) NULL);
				exit(3);
			}

			if (mybuf.FreePlace == 0) {
				puts("Жду освобождения места\n");
				if ((msgrcv(msqid, (struct mymsgbuf*) &mybuf, len, 2, 0) < 0)) {
					printf("Невозможно получить сообщение из очереди\n");
					fclose(DirtyDishes);
        				fclose(WasherProcess);	
					exit(4);
				}
			}
		}
	}
	
	
	
	fclose(DirtyDishes);
	fclose(WasherProcess);

	mybuf.DishesType = -1;
	mybuf.mtype = 1;
	if (msgsnd(msqid, (struct msgbuf*) &mybuf, len, 0) < 0) {
		printf("Невозможно отправить сообщение\n");
		msgctl(msqid, IPC_RMID, (struct msqid_ds*) NULL);
		exit(3);
	}
	return 0;
}
