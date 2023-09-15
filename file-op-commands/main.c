#include <stdio.h>
#include <stdlib.h>
#include "commands.h"

#define ROOT_PATH "E:/TestWorkSpace"
#define COMMANDS_COUNT 9

int main()
{
    char *COMMANDS [] =
                {"ls","cat","cp","rm","mv","mkdir","rmdir","cd","find"};

    char *get_com = (char *)malloc(100 * sizeof(char));
    char *cur_path = ROOT_PATH;
    struct file_in_dir *cur_dir_files_head = get_file_list(str_join(cur_path,"/*"));
    while(1){
        printf("%s",str_join(cur_path,"$ "));
        gets(get_com);
        get_com = str_format(get_com);

        int index = -1;
        if(str_cmp(get_com,"ls") || str_cmp(get_com,"find")){
            index = 0;
        }else{
            char *com_head = str_substring(get_com,0,str_indexOf(get_com,' ') - 1);
            index = get_array_index(COMMANDS,COMMANDS_COUNT,com_head);
        }

        switch (index)
        {
            case 0://run the ls
                cur_dir_files_head = get_file_list(str_join(cur_path,"/*"));
                ls(cur_dir_files_head);
                break;
            case 1://run the cat
                cat(cur_path,get_com);
                break;
            case 2://run the cp
                cp(cur_path,get_com);
                break;
            case 3://run the rm
                rm(cur_path,get_com);
                break;
            case 4://run the mv
                printf("run the mv\n");
                mv(cur_path,get_com);
                break;
            case 5://run the mkdir
                mkdir_myself(cur_path,get_com);
                break;
            case 6://run the rmdir
                rmdir_myself(cur_path,get_com);
                break;
            case 7://run the cd
                cur_path = cd(cur_path,get_com);
                break;
            case 8://run the find
                find(cur_path,get_com);
                break;
            default:
                printf("no command named %s\n",get_com);
                break;
        }
    }
    free(get_com);
    free(cur_dir_files_head);
    system("pause");
    return 0;
}
