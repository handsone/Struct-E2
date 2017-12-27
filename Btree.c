# include "Btree.h" 
int main (){
	btree_t * btree ;	
	btree_creat(&btree , 3);	
	char   ch ;
	cover();
	while(((ch = getchar()) != '0')){
		cover();
		switch(ch){
			case '1' :system("clear"); Input(btree);break ;
			case '2' :system("clear"); Output(btree);break ;
			case '3' :system("clear");deleteKey(btree);break;
			case '4' :system("clear");SearchKey(btree);break ;
			default :break;
		}
	}
	return 0  ;
}
