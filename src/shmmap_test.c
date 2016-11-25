#include"shm_map.h"
#include<stdio.h>

void print_iter(const char *k, const char *v){
	printf("(%s)=>(%s)\n", k, v);
}

int
main(){
	int cmd;
	char k[100], v[100];

	map_init(10000, 10000000, "shmmap.dat", NULL);
	while(true){
		printf("map size %d, operation: 1:put, 2:get, 3:iter, 4:free list, 5:free size, 6:exit\n", map_size());
		scanf("%d", &cmd);
		if(cmd == 6) break;
		switch(cmd){
			case 1:
				printf("key:\n");
				scanf("%s", k);
				printf("value:\n");
				scanf("%s", v);
				map_put(k, v);
				break;
			case 2:
				printf("key:\n");
				scanf("%s", k);
				printf("%s=>%s\n", k, map_get(k));
				break;
			case 3:
				map_iter(print_iter);
				break;
			case 4:
				m_free_list_info();
				break;
			case 5:
				printf("free size: %d\n", m_free_size());
				break;

		}
	}
}
