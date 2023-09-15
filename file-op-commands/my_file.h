#include <windows.h>
#include <direct.h>
#include "my_string.h" 
#define BUFFER_SIZE 1024
#define STRUCT_FILE_SIZE sizeof(struct file_in_dir)

struct file_in_dir{
    char *file_name;
    char file_type;
    unsigned int file_size;
    struct file_in_dir *next;
};

/*
    文件夹返回 'd'
    文件返回   'f'
 */
char file_type(char *name){
    if(strchr(name,'.')){
        return 'f';
    }else{
        return 'd';
    }
}

/*
    不存在返回 0
    存在返回 1
 */
int file_exist(char *file_path){
    if(file_path == NULL)
        return 0;
    return access(file_path,F_OK) + 1;
}
/*
    dir_path:文件目录
 */
struct file_in_dir *get_file_list(char *dir_path){
    struct _finddata_t fileInfo;
    struct file_in_dir *head, *flg;
    head = (struct file_in_dir *)malloc(STRUCT_FILE_SIZE);
    head->file_name = "head";
    head->next = NULL;
    flg = head;

    long fHandle;
    int fileCount = 0;
    if((fHandle = _findfirst(dir_path,&fileInfo)) == -1L){
        printf("路径错误!\n");
        return NULL;
    }else{
        do{
            if(strcmp(fileInfo.name,".") != 0 && strcmp(fileInfo.name, "..") !=0 ){
                fileCount++;
                struct file_in_dir *temp = (struct file_in_dir*)malloc(STRUCT_FILE_SIZE);
                temp->file_name = (char*)malloc(strlen(fileInfo.name)+1);
                strcpy(temp->file_name,fileInfo.name);
                temp -> file_type = file_type(fileInfo.name);
                temp -> file_size = fileInfo.size;
                temp -> next = NULL;

                head->next = temp;
                head = head->next;
            }
        }while(_findnext(fHandle,&fileInfo) == 0);
    }
    _findclose(fHandle);
    return flg;
}

void file_msg_log(struct file_in_dir * head){
    while(head->next){
        head = head->next;
        printf("%s\t %c\n",head->file_name,head->file_type);
    }
    printf("output finish\n");
}


void remove_empty_dir(char *file_path){
    if(file_path == NULL) return;
    char *file_name = get_fn_from_path(file_path);
    if(file_exist(file_path)){
        if(_rmdir(file_path) == 0){
            printf("文件 %s 删除成功!\n",file_name);
        }else{
            printf("文件 %s 删除失败!\n",file_name);
        }
    }else{
        printf("%s 不存在\n",file_name);
    }
}

void remove_file(char *file_path){
    if(file_path == NULL)
        return;
    char *file_name = get_fn_from_path(file_path);
    if(file_exist(file_path)){
        if(remove(file_path) == 0){
            printf("文件 %s 删除成功!\n",file_name);
        }else{
            printf("文件 %s 删除失败!\n",file_name);
        }
    }else{
        printf("文件 %s 不存在!\n",file_name);
    }
}

/*
    file_path: 文件路径
 */
void dir_remove_deep(char* file_path){
    if(file_path == NULL || file_type(file_path) == 'f'
                         || !file_exist(file_path)){
        printf("文件夹 %s 不存在!",get_fn_from_path(file_path));
        return;
    }

    struct file_in_dir *head = get_file_list(str_join(file_path,"/*"));
    if(head->next == NULL){
        //printf("当前文件夹 %s 为空!\n",get_fn_from_path(file_path));
        remove_empty_dir(file_path);
        return;
    }
    char *file_temp = str_join(file_path,"/");
    //对文件夹内容进行删除
    while(head->next){
        head = head->next;
        //printf("当前文件为:%s\t type:%c \n",head->file_name,head->file_type);
        if(head->file_type == 'f'){
            //printf("获取删除文件路径:%s\n",str_join(file_temp,head->file_name));
            remove_file(str_join(file_temp,head->file_name));
        }
        if(head->file_type == 'd'){
            //printf("获取当前文件夹路径:%s\n",str_join(file_temp,head->file_name));
            dir_remove_deep(str_join(file_temp,head->file_name));
        }
    }
    remove_empty_dir(file_path);
    free(head);
}

void create_dir(char *file_path){
    if(file_path == NULL){
        return;
    }
    if(!file_exist(file_path)){
        if(_mkdir(file_path) == 0){
            return;
        }
    }else{
        char y_n = 'n';
        printf("already exist! overwrite %s (y/n）？",file_path);
        if(y_n == 'y'){
            dir_remove_deep(file_path);
            create_dir(file_path);
        }
    }
}

void file_move(char *old_path,char *new_path){
    char *old_par_path = get_par_from_path(old_path);
    char *new_par_path = get_par_from_path(new_path);
    char *old_name = get_fn_from_path(old_path);

    if(strcmp(old_par_path,new_par_path) == 0){//相同父文件夹 重命名
        if(0 == rename(old_path,new_path)){
            printf("重命名成功!\n");
        }else{
            printf("重命名失败!\n");
        }
    }else{
        if(file_exist(new_path)){
            printf("mv: overwrite it? y or n:");
            char get_input = 'n';
            scanf("%c\n",&get_input);
            if(get_input == 'y'){
                if(file_type(old_name) == 'd'){
                    dir_remove_deep(new_path);
                }else{
                    remove_file(new_path);
                }
                rename(old_path,new_path);
            }
        }else if(rename(old_path,new_path) == 0){
            printf("移动成功!\n");
        }else{
            printf("移动失败!\n");
        }
    }
}

/*
    1 success
    0 fail
    src_path :源文件地址
    target_path :目标文件地址
 */
int file_copy(char *src_path,char *target_path){
    FILE* fin;
    FILE* fout;
    char *buf;
    char overwrite = 'y';
    int res = 1;
    if((fin = fopen(src_path,"r")) == NULL){
        return 0;
    }

    if(file_exist(target_path)){
        printf("cp :overwrite it (y/n)?");
        scanf("%c",&overwrite);
        if(overwrite == 'n'){
            return 0;
        }
    }
    //printf("开始复制\n");
    if((fout = fopen(target_path,"w")) == NULL){
        return 0;
    }
    if((buf = (char*)malloc(BUFFER_SIZE)) == NULL){
        return 0;
    }
    while(1){
        res = fread(buf,1,BUFFER_SIZE,fin);
        fwrite(buf,res,1,fout);
        if(feof(fin))
            break;
    }
    fclose(fin);
    fclose(fout);
    free(buf);
    return 1;
}
void file_read_by_line(char *path){
    if(file_exist(path)){
        FILE *fp;
        char buf[1024];
        if((fp = fopen(path,"r")) == NULL){
            printf("read error!\n");
            return;
        }else{
            while(!feof(fp)){
                if(fgets(buf,1024,fp) != NULL){
                    printf("%s\n",buf);
                }
            }
        }
        fclose(fp);
    }else{
        printf("file %s not exists!\n",get_fn_from_path(path));
    }
}

/*
    src_path :源目录地址
    target_path :目的目录地址
 */
void dir_copy(char *src_path,char *target_path){
    if(src_path == NULL || target_path == NULL
        || file_type(src_path) == 'f' || file_type(target_path) == 'f'){
        return;
    }
    if(file_exist(target_path)){
        printf("cp :overwirte '%s' (y/n)?",target_path);
        char get_y_n = 'y';
        scanf("%c",&get_y_n);
        if(get_y_n == 'n'){
            return;
        }else{
            dir_remove_deep(target_path);
        }
    }
    create_dir(target_path);//创建一个目标空文件夹
    struct file_in_dir *head = get_file_list(str_join(src_path,"/*"));
    if(head->next == NULL){ return;}
    char *src_t = str_join(src_path,"/");
    char *target_t = str_join(target_path,"/");
    while(head -> next != NULL){
        head = head -> next;
        if(head -> file_type == 'f'){
            file_copy(str_join(src_t,head ->file_name),
                      str_join(target_t,head ->file_name));
        }
        if(head -> file_type == 'd'){
            dir_copy(str_join(src_t,head -> file_name),
                    str_join(target_t,head -> file_name));
        }
    }
}
