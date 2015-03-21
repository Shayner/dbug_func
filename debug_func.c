#include<stdio.h>
#include<stdarg.h>
#include<libdwarf.h>
#include<libelf.h>
#include<dwarf.h>
#include<fcntl.h>
#include<sys/types.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
//char *filename="/home/shayne/file";
void die_printf(char *fmt,...){
	va_list ap;
	va_start(ap,fmt);
	vfprintf(stderr,fmt,ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}
int get_func_line(Dwarf_Debug dbg,Dwarf_Die the_die){
        Dwarf_Line *linebuf;
	Dwarf_Signed linecount;
	Dwarf_Unsigned lineno;
	Dwarf_Error error;
	Dwarf_Addr line_addr;
	int res;
	char *linesrc;
	res=dwarf_srclines(the_die,&linebuf,&linecount,&error);
	if(res==DW_DLV_NO_ENTRY)
		die_printf("Don't get the information of line!\n");
	//	return 2;
	else if(res==DW_DLV_OK){
		//printf("YESYESYES!!!\n");
		int i;
		for(i=0;i<linecount;i++){
			dwarf_lineno(linebuf[i],&lineno,&error);
			dwarf_lineaddr(linebuf[i],&line_addr,&error);
			printf("lineaddr:0x%llx,lineno:%lld\n",line_addr,lineno);
                      //  输出每一行所在的文件
		      //  dwarf_linesrc(linebuf[i],&linesrc,&error);
	              //  printf("the linesrc:%s\n",linesrc);
		}
		dwarf_srclines_dealloc(dbg,linebuf,linecount);
	}

	return 1;
}
void get_func_in_die(Dwarf_Debug dbg,Dwarf_Die the_die){
    char *die_name=0;
    const char *tag_name=0;
    Dwarf_Error error;
    Dwarf_Half tag;  //标记了属性的类型
    Dwarf_Unsigned account,lineno;
    Dwarf_Attribute *alist;
    Dwarf_Signed linecount;
    Dwarf_Line *linebuf;
    Dwarf_Addr lowpc,highpc;
    int rc_getaddr;
    int rc=dwarf_diename(the_die,&die_name,&error);
    if(rc==DW_DLV_ERROR)
	    die_printf("No the die name\n");
    else if(rc==DW_DLV_NO_ENTRY)
	    return ;
    //else printf("the die name: %s\n",die_name);
    if(dwarf_tag(the_die,&tag,&error)==DW_DLV_ERROR)
	    die_printf("Can't get the tag!!\n");
    
    if(tag!=DW_TAG_subprogram) return;

    if(dwarf_get_TAG_name(tag,&tag_name)==DW_DLV_ERROR)
	    die_printf("Can't get the tag name!\n");
    printf("the tag name:'%s'\n",die_name);
    rc_getaddr=dwarf_attrlist(the_die,&alist,&account,&error);
   
    if(rc_getaddr==DW_DLV_ERROR)
	    die_printf("Can't get the attrlist\n");
    int i;
   
    for( i=0;i<account;i++){
	    Dwarf_Half attr_tag;

	    if(dwarf_whatattr(alist[i],&attr_tag,&error)!=DW_DLV_OK)
		    die_printf("Error:Can't get the information\n");

 	    if(attr_tag==DW_AT_low_pc)
		   dwarf_formaddr(alist[i],&lowpc,&error);

 	    if(attr_tag==DW_AT_high_pc)
		   dwarf_formaddr(alist[i],&highpc,&error);

    }
	   printf("lowpc: 0x%llx\n",lowpc);
	   printf("highpc:0x%llx\n",highpc);
    //}
	 
}
void get_init_cu(Dwarf_Debug dbg){
	Dwarf_Unsigned cu_header_length=0;
	Dwarf_Unsigned abbrev_offset=0;
	Dwarf_Unsigned next_cu_header=0;
	Dwarf_Half version_stamp=0;
	Dwarf_Half address_size=0;
	Dwarf_Error error;
	Dwarf_Die no_die=0,child_die,cu_die;
       printf("Now Ready GO!!!\n");
	while(1){
		int rc;
		//获取头节点
	rc=dwarf_next_cu_header(
				dbg,
                                &cu_header_length,
				&version_stamp,
				&abbrev_offset,
				&address_size,
				&next_cu_header,
				&error);
	if(rc==DW_DLV_ERROR)
		die_printf("Wrong!!\n");
	else if(rc==DW_DLV_NO_ENTRY)
		break;

	if(dwarf_siblingof(dbg,no_die,&cu_die,&error)==DW_DLV_ERROR)
		die_printf("ERROR:getting the cu_die\n");
        int line=0;
	line=get_func_line(dbg,cu_die);
        printf("Return frome the line:%d\n",line);
	if(dwarf_child(cu_die,&child_die,&error)==DW_DLV_ERROR)
		die_printf("ERROR:getting the child_die\n");

	while(1){
		int res;
                get_func_in_die(dbg,child_die);
		res =dwarf_siblingof(dbg,child_die,&child_die,&error);
		if(res==DW_DLV_ERROR)
			die_printf("ERROR getting sibling of DIE\n");
		else if (res==DW_DLV_NO_ENTRY)
			break;

	}
   }
}
 int main(int argc, char** argv){
	 int fd;
	// char *pro_name=argv[1];
	 Dwarf_Debug dgb=0;
	 Dwarf_Error error;
	// pro_name=argv[1];
	 printf("%s\n",argv[1]);
	/* if(argc<=1){
                printf("Waring:please input the program's name!!!\n");
		return 1;
	 }*/
	if((fd=open(argv[1],O_RDONLY,0))<0){
		perror("open");
		//printf("Error:Can't open the file!\n");
		return 1;
	}

	if(dwarf_init(fd,DW_DLC_READ,0,0,&dgb,&error)!=DW_DLV_OK){
		printf("Waring:Can't open the prgram!!\n");
			return 1;
	}

	get_init_cu(dgb);

        if(dwarf_finish(dgb,&error)!=DW_DLV_OK){
		printf("Waring:Can't release the program!!\n");
		return 1;
	}
		close(fd);
		return 0;
 }
