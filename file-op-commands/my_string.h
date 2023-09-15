#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    -1 NULL
 */
int str_len(char *str){
    int len = -1;
    if(str == NULL){
        return len;
    }
    len = 0;
    while(*(str + len) != '\0'){
        //printf("%c\t",*(str+len));
        len++;
    }
    //printf("get the length is %d\n",len);
    return len;
}
/*
    0 不等
    1 相等
 */
int str_cmp(char *s1,char *s2){
    int len1 = str_len(s1);
    int len2 = str_len(s2);
    if(len1 != len2){
        return 0;
    }
    if(len1 == len2 && len1 != -1){
        while(len1 != -1){
            if(*(s1 + len1) != *(s2 + len1)){
                return 0;
            }
            len1--;
        }
        return 1;
    }
    return 0;
}

char* delete_char(char* a,char del){
    char* new_str = (char*)malloc(strlen(a)+1);
    char* flg = new_str;
    while(*a != '\0'){
        if(*a != del){
            *new_str = *a;
            new_str++;
        }
        a++;
    }
    *new_str = '\0';
    return flg;
}
/*
    字符串格式化：收尾去空格 中间多空格只保留一个
 */
char* str_format(char *src){
    if(src == NULL){
        return NULL;
    }
    char *res = (char*)malloc(strlen(src) + 1);
    char *flg = res;
    int touch = 0;
    int t = 0;
    while(*(src + t) != '\0'){
        if(!touch && *(src + t) != ' ') {
            touch = 1;
        }
        if(touch == 1){
            *res = *(src + t);
            t++;
            res++;
            if(*(src + t - 1) == ' '){
                while(*(src + t) != '\0'){
                    if(*(src + t) == ' '){
                        t++;
                    }else{
                        break;
                    }
                }
            }
        }else{
            t++;
        }
    }
    *res = '\0';
    return flg;
}
char *str_join(char *s1,char *s2){//字符串合并
    int len = str_len(s1) + str_len(s2) + 1;
    char *result = (char *)malloc(len);
    char *temp = result;
    while(*s1 != '\0'){
        *result = *s1;
        result++;
        s1++;
    }
    while(*s2 != '\0'){
        *result = *s2;
        result++;
        s2++;
    }
    *result = '\0';
    return temp;
}

char *str_substring(char *src,int s,int t){
    if(src == NULL){
        return NULL;
    }
    int len = strlen(src);
    if(s<0 || t > (len-1) || s > t){
        return NULL;
    }else{
        char *res = (char*)malloc(t - s + 2);
        char *flg = res;
        while(s <= t){
            *res = *(src + s);
            res++;
            s++;
        }
        *res = '\0';
        return flg;
    }
}

int str_indexOf(char *src,char c){
    if(src == NULL)
        return -1;
    int start = 0;
    while(*(src+start) != '\0'){
        if(*(src + start) == c){
            return start;
        }
        start++;
    }
    return -1;
}
int str_indexOf_start(char *src,char c,int start){
    if(src == NULL){
        return -1;
    }

    if(start < 0){
        start = 0;
    }
    int len = str_len(src);
    while(start < len){
        if(*(src + start) == c){
            return start;
        }
        start++;
    }
    return -1;

}

int str_lastIndexOf(char *src,char c){
    if(src == NULL)
        return -1;
    int len = strlen(src);
    while(len > 0){
        len--;
        if(*(src + len) == c){
            return len;
        }
    }
    return -1;
}

int str_constain_char(char *str,char c){
    int flg = 0;
    int len = str_len(str);
    int counter = 0;
    while(flg < len){
        if(*(str + flg) == c){
            counter++;
        }
        flg++;
    }
    return counter;
}
char* get_fn_from_path(char* path){
    char* file_name = strrchr(path,'/');
    return (file_name + 1);
}

char* get_par_from_path(char* path){
    if(path != NULL){
        int tail = str_lastIndexOf(path,'/');
        return str_substring(path,0,tail - 1);
    }else{
        return NULL;
    }
}
int get_array_index(char **arr,int length,char *str){
    int res = 0;
    while(res<length){
        if(str_cmp(arr[res],str)){
            return res;
        }
        res++;
    }
    return -1;
}

/*
    0 不是
    1 是
 */
int str_full_path(char *path){
    int index = str_indexOf(path,':');
    if(index == -1){
        return 0;
    }else{
        return 1;
    }
}