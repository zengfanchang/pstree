#include <stdio.h>
#include <assert.h>
#include <glob.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <getopt.h>

struct node {
  int pid, ppid;
  char name[32];
  struct node *next, *pre, *child;
};

typedef struct node Tnode;
typedef struct node Lnode;
Tnode *root;
Lnode head;

// start 函数声明
void deal_args(int argc, char *argv[], int flags[]);
void print_flags(int flags[]);
void print_version(int flags[]);
void read_info(glob_t *pglob, char *pattern);
void creat_node(Tnode *temp, char *path);
void create_list(Lnode *head, Lnode *temp, int flag);
void insert(Lnode *head, Lnode *temp, int flag);
void create_tree(int flags[]);
void print_tree(Tnode *root, int wid);
// end

void deal_args(int argc, char *argv[], int flags[] ){
  if (argc == 1) {return ;}
  char *optstring = "Vpn";
  struct option opts[] = {
    {"version", no_argument, NULL, 'V'},
    {"show-pids", no_argument, NULL, 'p'},
    {"numeric-sort", no_argument, NULL, 'n'},
    {0, 0, 0, 0}
  };
  int opt;
  while ((opt = getopt_long(argc, argv, optstring, opts, NULL)) != -1) {
    if (opt == '?') {
      printf("Usage: %s [-V|--version] [-p|--show-pids] [-n|--numeric-sort]\n", argv[0]);
      exit(1); 
    }
    switch (opt) {
      case 'V': flags[1] = 1; break;
      case 'p': flags[2] = 1; break;
      case 'n': flags[3] = 1; break;
      default:  break;
    }
  }
  printf("%c\n", opt);
}

void print_flags(int flags[]) {
  for (int i = 1; i < 4; i++) {
    printf("flags[%d] = %d\n", i, flags[i]);
  }
}

void print_version(int flags[]) {
  if (flags[1]) {
    FILE *fp = fopen("./version", "r");
    if (!isatty(fileno(stdout))) {
      dup2(fileno(stdin), 1);
    } 
    while (!feof(fp)) {
      char ch = fgetc(fp);
      printf("%c", ch);
    }
    if (!isatty(fileno(stdout))) {
      dup2(fileno(stdin), 1);
    }
    fclose(fp);
    exit(0);
  }
}

void read_info(glob_t *pglob, char *pattern) {
  int ret = glob(pattern, GLOB_ONLYDIR, NULL, pglob);
  if (ret != 0) {
    printf("glob error\n");
    exit(0);
  }
  for (int i = 0; i < pglob->gl_pathc; i++) {
    Lnode *temp = (Lnode *)malloc(sizeof(Lnode));
    memset(temp, 0, sizeof(Lnode));
    creat_node(temp, pglob->gl_pathv[i]);
    create_list(&head, temp, 1); // 默认从小到大排序
  }
}

void creat_node(Tnode *temp, char *path) {
  FILE *fp = fopen(path, "r");
  if (!fp) {
    printf("open file error\n");
    exit(0);
  }
  int i = 0, flag = 4;
  char buff[1024] = {0}, *p;
  fgets(buff, 1024, fp);
  p = buff;
  while (flag == 4) {
    if (*p == ' ') {
      flag--;
    } else {
      temp->pid = temp->pid * 10 + *p - '0';
    }
    p++;
  }
  
  while (flag == 3) {
    if (*p == ' ') {
      flag--;
    } else if (*p != '(' && *p != ')') {
      temp->name[i++] = *p;
    }
    p++;
  }
  while (flag == 2) {
    if (*p == ' ') {
      flag--;
    }
    p++;
  }

  while (flag == 1) {
    if (*p == ' ') {
      flag--;
    } else {
      temp->ppid = temp->ppid * 10 + *p - '0';
    }
    p++;
  }
  fclose(fp);
}

void create_list(Lnode *head, Lnode *temp, int flag) {
  insert(head, temp, flag);
}

void insert(Lnode *head, Lnode *temp, int flag) { // 链表插入
  if (head->pre == NULL && head->next == NULL) { // 插入第一个兄弟节点
      head->next = temp;
      temp->pre = head;
      temp->next = NULL;

  } else if (flag == 1) { // 按进程大小排序
    while (head->pid < temp->pid && head->next != NULL ) {
      head = head->next;
    }
    if (head->pid > temp->pid) { // 插入前面
      temp->next = head;
      temp->pre = head->pre;
      head->pre->next = temp;
      head->pre = temp;
    } else { // 插入后面,后面只能为空
      temp->next = head->next;
      temp->pre = head;
      head->next = temp;
    }
  } else { // 字母排序
    while (strcmp(head->name, temp->name) < 0 && head->next != NULL) { // 大数排在后面
      head = head->next;
    }
    if (strcmp(head->name, temp->name) > 0) { // 插入前面 
      temp->next = head;
      temp->pre = head->pre;
      if (head->pre != NULL) { // 前面
        head->pre->next = temp;
      }
      head->pre = temp;
    } else { // 插入后面,
      temp->next = head->next;
      temp->pre = head;
      if (head->next != NULL) {
        head->next->pre = temp;
      }
      head->next = temp;
    }
  }
}

void find_Tnode(Tnode *root, Tnode *temp, int pid) {
  if (root == NULL) {return ;}
  if (root->pid == pid) {
    temp = root;
  }
  find_Tnode(root->child, temp, pid);
  find_Tnode(root->next, temp, pid);
}

void printf_list(Lnode *head) {
  // 打印节点
  if (head == NULL) {return ;}
  // print_node(head, sizeof(head->name));
  printf("%s(%d)%d\n", head->name, head->pid, head->ppid);
  printf_list(head->next);
}

void print_node(Lnode *head, int wid) {
  if (head->pre == NULL)
    printf("%s ", head->name);
  else { // 兄弟节点
    printf("%*s ", (int)(wid + strlen(head->name)), head->name);
  }
  if (head->child == NULL) {
    printf("\n");
  }
}

void create_Tnode(Tnode *root, Lnode *head, int flag) { // head为链表的单个,单次递归是不能生成一颗树的
  if (root == NULL || head == NULL) {return ;}
  if (root->pid == head->ppid) { // 是子节点
    if (root->child == NULL) { // 没有孩子
      root->child = head;
      head->pre = NULL;
      if (head->next != NULL) {
        head->next->pre = NULL;
      }
      head->next = NULL;
    } else { // 有孩子
      Tnode *temp = root->child;
      if (flag) { // id号排序
        while (temp->next != NULL && head->pid < temp->pid) { // 比较方法有问题
          temp = temp->next;
        }
        insert(temp, head, flag); // 插入链表中（单链表和双链表）
      } else { // 字母排序
        if (temp->pre == NULL && (strcmp(temp->name, head->name)) > 0) { // 插入上面
          head->next = temp;
          temp->pre = head;
          head->pre = NULL;
          root->child = head;
        } else { // 插入下面
          insert(temp, head, flag); // 插入链表中（单链表和双链表）
        }
      }

      head = NULL;// 一种为NULL, 置空
      return ; // 分支限界
    }
  }
  create_Tnode(root->next, head, flag);
  create_Tnode(root->child, head, flag);
}

void create_tree(int flags[]) {
  for (Tnode *p = head.next; p != NULL;) {
    Tnode *temp = p->next;
    if (flags[2]) // 输出格式
      sprintf(p->name, "%s(%d)", p->name, p->pid); 
    create_Tnode(root, p, flags[3]); // 二级指针
    p = temp;
  }
}

void print_tree(Tnode *root, int wid) { // 打印树
  if (root == NULL) {return ;}
  // 同一级的不需要加输出
  print_node(root, wid);
  print_tree(root->child, wid + strlen(root->name) + 1);
  print_tree(root->next, wid);
}


  

int main(int argc, char *argv[]) {
  int flags[4] = {0}; // 处理参数
  deal_args(argc, argv, flags);
  char *p = "/proc/[0-9]*/stat";
  glob_t pglob;
  Tnode zero;
  memset(&zero, 0, sizeof(Tnode));
  root = &zero;
  if (flags[2]) {
    sprintf(zero.name, "%s", "?()");
  } else {
    sprintf(zero.name, "%s", "?");
  }
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  print_version(flags);
  read_info(&pglob, p);
  // printf_list(&head);
  create_tree(flags);
  print_tree(root, 0);
  assert(!argv[argc]);
  return 0;
}
