#include "my_file.h"

void ls(struct file_in_dir* head){
    if(head == NULL || head -> next == NULL){
        printf("NULL\n");
    }else{
        while(head->next != NULL){
            head = head->next;
            printf("%s\t",head -> file_name);
        }
        free(head);
        printf("\n");
    }
}


void cp(char *cur_path,char *commands){
    int f1 = str_indexOf(commands,' ');
    int f2 = str_indexOf_start(commands,' ',f1 + 1);
    char *src = str_substring(commands,f1 + 1,f2 - 1);
    char *target = str_substring(commands,f2 + 1,strlen(commands) -1);

    cur_path = str_join(cur_path,"/");
    if(!str_full_path(src)){
        src = str_join(cur_path,src);
    }
    if(!file_exist(src)){
        return;
    }
    if(*(target + strlen(target) -1) == '/'){
        char *fn =get_fn_from_path(src);
        target = str_join(target,fn);
    }
    if(!str_full_path(target)){
        target = str_join(cur_path,target);
    }

    printf("%s\t target = %s\n",src,target);
    if(file_type(src) == 'f'){
        file_copy(src,target);
    }else{
        dir_copy(src,target);
    }
    free(src);
    free(target);
}

void mv(char *cur_path,char *commands){
    int f1 = str_indexOf(commands,' ');
    int f2 = str_indexOf_start(commands,' ',f1 + 1);
    char *src = str_substring(commands,f1 + 1,f2 - 1);
    char *target = str_substring(commands,f2 + 1,strlen(commands) -1);
    cur_path = str_join(cur_path,"/");
    if(!str_full_path(src)){
        src = str_join(cur_path,src);
    }
    if(!file_exist(src)){
        return;
    }
    if(*(target + strlen(target) -1) == '/'){
        char *fn =get_fn_from_path(src);
        target = str_join(target,fn);
    }
    if(!str_full_path(target)){
        target = str_join(cur_path,target);
    }
    printf("src_path: %s\t target_path: %s\n",src,target);
    file_move(src,target);
    free(src);
    free(target);
}

/*
    主要针对 rm，rmdir,find 的目的地址
 */
char *two_init_path(char*cur_path,char*commands){
    int index = str_indexOf(commands,' ');
    char *dir_path = (commands + index + 1);
    if(!str_full_path(dir_path)){
        cur_path = str_join(cur_path,"/");
        dir_path = str_join(cur_path,dir_path);
    }
    return dir_path;
}

void find(char *cur_path,char *commands){
    char *file_path = two_init_path(cur_path,commands);
    struct file_in_dir* files = get_file_list(file_path);
    ls(files);
}

void rm(char *cur_path,char *commands){
    char *file_path = two_init_path(cur_path,commands);
    if(!file_exist(file_path)){
        printf("rm: %s 不存在\n",file_path);
        return;
    }
    remove_file(file_path);
    free(file_path);
}


void mkdir_myself(char *cur_path,char *commands){
    char *dir_path = two_init_path(cur_path,commands);
    create_dir(dir_path);
    free(dir_path);
}

void rmdir_myself(char *cur_path,char *commands){
    char *dir_path = two_init_path(cur_path,commands);
    dir_remove_deep(dir_path);
    free(dir_path);
}


char* cd(char *cur_path,char *commands){
    if(str_cmp(commands + 3,"..")){//返回上层目录
        int tail = str_lastIndexOf(cur_path,'/');
        return str_substring(cur_path,0,tail - 1);
    }else if(str_cmp(commands + 3,"/")){//进入根目录
        int tail = str_indexOf(cur_path,'/');
        if(tail != -1){
            return str_substring(cur_path,0,tail - 1);
        }else{
            return cur_path;
        }
    }else{
        char *file_temp = str_join(cur_path,"/");
        file_temp = str_join(file_temp,(commands + 3));
        if(file_exist(file_temp) == -1){
            printf("cd: %s 目录不存在\n",(commands+3));
            return cur_path;
        }else{
            return file_temp;
        }
    }
}

char *path_full(char *cur_path,char *temp_path){
    if(!str_full_path(temp_path)){
        cur_path = str_join(cur_path,"/");
        temp_path = str_join(cur_path,temp_path);
    }
    return temp_path;
}


void cat(char *cur_path,char*commands){
     int space_count = str_constain_char(commands,' ');
     cur_path = str_join(cur_path,"/");
     if(space_count == 1){
        int index = str_indexOf(commands,' ');
        if(!str_full_path(commands + index +1)){
            cur_path = str_join(cur_path,(commands + index + 1));
        }
        file_read_by_line(cur_path);
     }else if(space_count == 4){
        int f1 = str_indexOf(commands,' ');
        int f2 = str_indexOf_start(commands,' ',f1 + 1);
        int f3 = str_indexOf_start(commands,' ',f2 + 1);
        int f4 = str_indexOf_start(commands,' ',f3 + 1);
        char *path1 = str_substring(commands,f1 + 1,f2 - 1);
        char *path2 = str_substring(commands,f2 + 1,f3 - 1);
        char *path3 = commands + f4 + 1;
        //printf("%s\t%s\t%s\n",path1,path2,path3);
        if(!str_full_path(path1)){
            path1 = str_join(cur_path,path1);
        }
        if(!str_full_path(path2)){
            path2 = str_join(cur_path,path2);
        }
        if(!str_full_path(path3)){
            path3 = str_join(cur_path,path3);
        }

        if(file_type(path1) != 'f' || file_type(path2) != 'f'
                                   || file_type(path3) != 'f'
                                   || !file_exist(path1)
                                   || !file_exist(path2)){
            return;
        }
        FILE* fin;
        FILE* fout;
        char *buf;
        int res = 1;
        if((fin = fopen(path1,"r")) == NULL || (fout = fopen(path3,"a")) == NULL
                                            || (buf = (char*)malloc(1024)) == NULL){
            return;
        }
        while(1){
            res = fread(buf,1,BUFFER_SIZE,fin);
            fwrite(buf,res,1,fout);
            if(feof(fin)){
                break;
            }
        }
        fclose(fin);
        fin = fopen(path2,"r");
        while(1){
            res = fread(buf,1,BUFFER_SIZE,fin);
            fwrite(buf,res,1,fout);
            if(feof(fin)){
               break;
            }
        }
        fclose(fin);
        fclose(fout);
        free(buf);
     }
}
