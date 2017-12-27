# include <stdio.h>
# include <errno.h>
# include <string.h>
# include <malloc.h>
# include <stdlib.h>

# define OVERFLOW -2 
# define TRUE      1
# define FALSE     0
# define OK        1
# define ERROR     0
# define INFEASIBLE -1
# define STACK_INIT_SIZE 100
# define STACKINCREMENT  10 
//节点结构
typedef struct _btree_node_t  
{  
	int num;                        /* 关键字个数 */  
	int *key;                       /* 关键字：所占空间为(max+1) - 多出来的1个空间用于交换空间使用 */  
	struct _btree_node_t **child;   /* 子结点：所占空间为（max+2）- 多出来的1个空间用于交换空间使用 */  
	struct _btree_node_t *parent;   /* 父结点 */  
}btree_node_t;  

typedef btree_node_t *   SElemType ;
//B树结构
typedef struct  
{  
	int max;                        /* 单个结点最大关键字个数 - 阶m=max+1 */  
	int min;                        /* 单个结点最小关键字个数 min = m / 2 向上取整*/
	int sidx;                       /* 分裂索引 = (max+1)/2 */  
	btree_node_t *root;             /* B树根结点地址 */  
}btree_t;  

typedef int Status     ;
Status Search(btree_t * btree, int key );
void deleteKey(btree_t * btree);
void PrintfLastInfor(btree_node_t * node , int lev);
Status leaf(btree_node_t * node);
void cover(void);
void Input(btree_t * btree);
int btree_delete(btree_t *btree, int key);
int _btree_delete(btree_t *btree, btree_node_t *node, int idx);
void InOder(SElemType node, int lev);
int btree_merge(btree_t *btree, btree_node_t *node);
int _btree_merge(btree_t *btree, btree_node_t *left, btree_node_t *right, int mid);
int btree_creat(btree_t **_btree, int m);
int btree_insert(btree_t *btree, int key) ;
int _btree_insert(btree_t *btree, btree_node_t *node, int key, int idx);
int btree_split(btree_t *btree, btree_node_t *node);
btree_node_t * btree_creat_node(btree_t *btree);
void   PrintfInfor(btree_node_t * node, int lev );
void mib_tree_dump(btree_t * Btree);

int btree_creat(btree_t **_btree, int m)  
{  
	btree_t *btree = NULL;  

	if(m < 3) {  
		printf( "Parameter 'max' must geater than 2.\n" );  
		return -1;  
	}  

	btree = (btree_t *)calloc(1, sizeof(btree_t));  //calloc 动态分配空间，并置零
	if(NULL == btree) {  
		fprintf(stderr, "[%s][%d] errmsg:[%d] %s!\n", __FILE__, __LINE__, errno, strerror(errno));  
		return -1;  
	}  

	btree->max= m - 1;  
	btree->min = m/2;  
	if(0 != m%2) {  
		btree->min++;  
	}  
	btree->min--;  
	btree->sidx = m/2;  
	btree->root = NULL; /* 空树 */  

	*_btree = btree;  
	return 0;  
}  

int btree_insert(btree_t *btree, int key)  
{  
	int idx = 0;  
	btree_node_t *node = btree->root;  

	/* 1. 构建第一个结点 */  
	if(NULL == node) {  
		node = btree_creat_node(btree);  
		if(NULL == node) {  
			fprintf(stderr, "[%s][%d] Create node failed!\n", __FILE__, __LINE__);  
			return -1;  
		}  

		node->num = 1;   
		node->key[0] = key;  
		node->parent = NULL;  

		btree->root = node;  
		return 0;  
	}  

	/* 2. 查找插入位置  这里和查找算法相似*/
	while(NULL != node) {  
		for(idx=0; idx<node->num; idx++) {  
			if(key == node->key[idx]) {  
				fprintf(stderr, "[%s][%d] The node is exist!\n", __FILE__, __LINE__);  
				return 0;  
			}  
			else if(key < node->key[idx]) {  
				break;  
			}  
		}  

		if(NULL != node->child[idx]) {  
			node = node->child[idx];  
		}  
		else {  
			break;  
		}  
	}  

	/* 3. 执行插入操作 */  
	return _btree_insert(btree, node, key, idx);  
}  

int _btree_insert(btree_t *btree, btree_node_t *node, int key, int idx)  
{  
	int i = 0;  

	/* 1. 移动关键字:首先在最底层的某个非终端结点上插入一个关键字,因此该结点无孩子结点，故不涉及孩子指针的移动操作 */  
	for(i=node->num; i>idx; i--) {  
		node->key[i] = node->key[i-1];  
	}  

	node->key[idx] = key; /* 插入 */  
	node->num++;  

	/* 2. 分裂处理 */  
	if(node->num > btree->max) {  
		return btree_split(btree, node);  
	}  

	return 0;  
}  


int btree_split(btree_t *btree, btree_node_t *node)  
{  
	int idx = 0, total = 0, sidx = btree->sidx;  
	btree_node_t *parent = NULL, *node2 = NULL;   


	while(node->num > btree->max) {  
		/* Split node */   
		total = node->num;  

		node2 = btree_creat_node(btree);  
		if(NULL == node2) {         
			fprintf(stderr, "[%s][%d] Create node failed!\n", __FILE__, __LINE__);  
			return -1;  
		}  

		/* Copy data */   
		memcpy(node2->key, node->key + sidx + 1, (total-sidx-1) * sizeof(int));  
		memcpy(node2->child, node->child+sidx+1, (total-sidx) * sizeof(btree_node_t *));  

		node2->num = (total - sidx - 1);  
		node2->parent  = node->parent;  

		node->num = sidx;   
		/* Insert into parent */  
		parent  = node->parent;  
		if(NULL == parent)  {         
			/* Split root node */   
			parent = btree_creat_node(btree);  
			if(NULL == parent) {         
				fprintf(stderr, "[%s][%d] Create root failed!", __FILE__, __LINE__);  
				return -1;  
			}         

			btree->root = parent;   
			parent->child[0] = node;   
			node->parent = parent;   
			node2->parent = parent;   

			parent->key[0] = node->key[sidx];  
			parent->child[1] = node2;  
			parent->num++;  
		}         
		else {         
			/* Insert into parent node */   
			for(idx=parent->num; idx>0; idx--) {         
				if(node->key[sidx] < parent->key[idx-1]) {         
					parent->key[idx] = parent->key[idx-1];  
					parent->child[idx+1] = parent->child[idx];  
					continue;  
				}  
				break;  
			}         

			parent->key[idx] = node->key[sidx];  
			parent->child[idx+1] = node2;  
			node2->parent = parent;   
			parent->num++;  
		}         

		memset(node->key+sidx, 0, (total - sidx) * sizeof(int));  
		memset(node->child+sidx+1, 0, (total - sidx) * sizeof(btree_node_t *));  

		/* Change node2's child->parent */  
		for(idx=0; idx<=node2->num; idx++) {  
			if(NULL != node2->child[idx]) {         
				node2->child[idx]->parent = node2;  
			}         
		}         
		node = parent;   
	}  

	return 0;  
}  

btree_node_t *btree_creat_node(btree_t *btree)  
{  
	btree_node_t *node = NULL;  

	node = (btree_node_t *)calloc(1, sizeof(btree_node_t));  
	if(NULL == node) {  
		fprintf(stderr, "[%s][%d] errmsg:[%d] %s\n", __FILE__, __LINE__, errno, strerror(errno));  
		return NULL;  
	}  

	node->num = 0;  

	/* More than (max) is for move */  
	node->key = (int *)calloc(btree->max+1, sizeof(int));  
	if(NULL == node->key) {  
		free(node), node=NULL;  
		fprintf(stderr, "[%s][%d] errmsg:[%d] %s\n", __FILE__, __LINE__, errno, strerror(errno));  
		return NULL;  
	}  

	/* More than (max+1) is for move */  
	node->child = (btree_node_t **)calloc(btree->max+2, sizeof(btree_node_t *));  
	if(NULL == node->child) {  
		free(node->key);  
		free(node), node=NULL;  
		fprintf(stderr, "[%s][%d] errmsg:[%d] %s\n", __FILE__, __LINE__, errno, strerror(errno));  
		return NULL;  
	}  

	return node;  
}  
void  PrintfInfor(btree_node_t * node , int lev ){
	int *  p = node ->key;
	for (int j = 0 ; j < lev -1; j ++){
		printf("|-------");
	}
	printf("<%d|",  node->num);
	for( int i =0 ; i < node ->num ; i ++ ){
		printf(" %d" , p[i]);
	}
	printf(" >\n");
}
void InOder(SElemType node , int lev ){
	if(node != NULL){
		lev ++ ;
		PrintfInfor(node, lev);
		for(int i = 0 ; i <=  node ->num  ; i++){
			InOder(node->child[i], lev );
		}
		if(!leaf(node)){
			PrintfLastInfor(node,lev);
		}

	}
}

int btree_delete(btree_t *btree, int key)  
{  
    int idx = 0;  
    btree_node_t *node = btree->root;  
  
  
    while(NULL != node) {  
        for(idx=0; idx<node->num; idx++) {  
            if(key == node->key[idx]) {  
                return _btree_delete(btree, node, idx);  
            }  
            else if(key < node->key[idx]) {  
                break;  
            }  
        }  
  
        node = node->child[idx];  
    }  
  
    return 0;  
}  

int _btree_delete(btree_t *btree, btree_node_t *node, int idx)  
{  
    btree_node_t *orig = node, *child = node->child[idx];  
  
    /* 使用node->child[idx]中的最大值替代被删除的关键字 */  
    while(NULL != child) {  
        node = child;  
        child = node->child[child->num];  
    }  
  
    orig->key[idx] = node->key[node->num - 1];  
  
    /* 最终其处理过程相当于是删除最底层结点的关键字 */  
    node->key[--node->num] = 0;  
    if(node->num < btree->min) {  
        return btree_merge(btree, node);  
    }  
  
    return 0;  
}  

int btree_merge(btree_t *btree, btree_node_t *node)  
{  
    int idx = 0, m = 0, mid = 0;  
    btree_node_t *parent = node->parent, *right = NULL, *left = NULL;  
  
    /* 1. node是根结点, 不必进行合并处理 */  
    if(NULL == parent) {  
        if(0 == node->num) {  
            if(NULL != node->child[0]) {  
                btree->root = node->child[0];  
                node->child[0]->parent = NULL;  
            }  
            else {  
                btree->root = NULL;  
            }  
            free(node);  
        }  
        return 0;  
    }  
  
    /* 2. 查找node是其父结点的第几个孩子结点 */  
    for(idx=0; idx<=parent->num; idx++) {  
        if(parent->child[idx] == node) {  
            break;  
        }  
    }  
  
    if(idx > parent->num) {  
        fprintf(stderr, "[%s][%d] Didn't find node in parent's children array!\n", __FILE__, __LINE__);  
        return -1;  
    }  
    /* 3. node: 最后一个孩子结点(left < node) 
     * node as right child */  
    else if(idx == parent->num) {  
        mid = idx - 1;  
        left = parent->child[mid];  
  
        /* 1) 合并结点 */  
        if((node->num + left->num + 1) <= btree->max) {  
            return _btree_merge(btree, left, node, mid);  
        }  
  
        /* 2) 借用结点:brother->key[num-1] */  
        for(m=node->num; m>0; m--) {  
            node->key[m] = node->key[m - 1];  
            node->child[m+1] = node->child[m];  
        }  
        node->child[1] = node->child[0];  
  
        node->key[0] = parent->key[mid];  
        node->num++;  
        node->child[0] = left->child[left->num];  
        if(NULL != left->child[left->num]) {  
            left->child[left->num]->parent = node;  
        }  
  
        parent->key[mid] = left->key[left->num - 1];  
        left->key[left->num - 1] = 0;  
        left->child[left->num] = NULL;  
        left->num--;  
        return 0;  
    }  
      
    /* 4. node: 非最后一个孩子结点(node < right) 
     * node as left child */  
    mid = idx;  
    right = parent->child[mid + 1];  
  
    /* 1) 合并结点 */  
    if((node->num + right->num + 1) <= btree->max) {  
        return _btree_merge(btree, node, right, mid);  
    }  
  
    /* 2) 借用结点: right->key[0] */  
    node->key[node->num++] = parent->key[mid];  
    node->child[node->num] = right->child[0];  
    if(NULL != right->child[0]) {  
        right->child[0]->parent = node;  
    }  
  
    parent->key[mid] = right->key[0];  
    for(m=0; m<right->num; m++) {  
        right->key[m] = right->key[m+1];  
        right->child[m] = right->child[m+1];  
    }  
    right->child[m] = NULL;  
    right->num--;  
    return 0;  
}  

int _btree_merge(btree_t *btree, btree_node_t *left, btree_node_t *right, int mid)  
{  
    int m = 0;  
    btree_node_t *parent = left->parent;  
  
    left->key[left->num++] = parent->key[mid];  
  
    memcpy(left->key + left->num, right->key, right->num*sizeof(int));  
    memcpy(left->child + left->num, right->child, (right->num+1)*sizeof(btree_node_t *));  
    for(m=0; m<=right->num; m++) {  
        if(NULL != right->child[m]) {  
            right->child[m]->parent = left;  
        }  
    }  
    left->num += right->num;  
  
    for(m=mid; m<parent->num-1; m++) {  
        parent->key[m] = parent->key[m+1];  
        parent->child[m+1] = parent->child[m+2];  
    }  
  
    parent->key[m] = 0;  
    parent->child[m+1] = NULL;  
    parent->num--;  
    free(right);  
  
    /* Check */  
    if(parent->num < btree->min) {  
        return btree_merge(btree, parent);  
    }  
  
    return 0;  
}  
void Input(btree_t * btree){
	int m ;
	printf("插入创建B树，请输入一串数字序列  以-1为结束符\n");
	scanf("%d", &m);
	while(m != -1){
		btree_insert(btree , m);
		scanf("%d",&m);
	}
}
void Output(btree_t * btree){
	int lev = 0 ;
	printf("调用递归遍历：\n");
	InOder(btree -> root , lev );
}
void deleteKey(btree_t * btree){
	int m ; 
	printf("删除指定关键字: ");
	scanf("%d", &m);
	btree_delete(btree , m);
	Output(btree);
}
Status SearchKey(btree_t * btree ){
	int  m ;
	printf("请输入要查询的数字");
	scanf("%d",&m);
	Search(btree , m);
	return OK ;
}
Status Search(btree_t * btree, int key ){
	btree_node_t  * node = btree -> root ;
	int idx ;
	while(NULL != node) {  
		for(idx=0; idx<node->num; idx++) {  
			if(key == node->key[idx]) {  
				printf("The node is exist!\n" );  
				return OK ;
			}  
			else if(key < node->key[idx]) {  
				break;  
			}  
		}  

		if(NULL != node->child[idx]) {  
			node = node->child[idx];  
		}  
		else {  
			printf("The node is not exist!\n");
			return OK ;
		}  

	}  
	return OK ;
}

void cover(void){
	puts("--------------------------------");
	puts("--------------------------------");
	puts("--------------------------------");
	puts("-----1.Input -------------------");
	puts("-----2.Output-------------------");
	puts("-----3.Delete-------------------");
	puts("-----4.Search-------------------");
	puts("--------------------------------");
	puts("--------------------------------");
	puts("--------------------------------");
	puts("--------------------------------");
	puts("--------------------------------");
}

Status leaf(btree_node_t * node){
	if(node ->child[0] == NULL){
		return TRUE ;
	}
	return FALSE ;
}

void PrintfLastInfor(btree_node_t * node , int lev){

	int *  p = node ->key;
	for (int j = 0 ; j < lev -1; j ++){
		printf("|-------");
	}
	printf("</%d|",  node->num);
	for( int i =0 ; i < node ->num ; i ++ ){
		printf(" %d" , p[i]);
	}
	printf(" >\n");
}
