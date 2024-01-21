// SPDX-License-Identifier: LGPL-2.1+
/*
 * Copyright 2024 BigfootACA <bigfoot@classfun.cn>
 */

#include<err.h>
#include<time.h>
#include<errno.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<inttypes.h>
#include<blkid/blkid.h>
#include<sys/stat.h>
#include<sys/ioctl.h>
#include<sys/sendfile.h>
#include<linux/fs.h>
#include<linux/magic.h>

const char*size_units_ib[]={"B","KiB","MiB","GiB","TiB","PiB","EiB","ZiB","YiB",NULL};
#define RET_ERRNO(e) do{errno=(e);return -1;}while(0)
#define WARN_ERRNO(e,fmt...) do{warnx(fmt);RET_ERRNO(e);}while(0)
#define WARN_RET(fmt...) do{warn(fmt);return -1;}while(0)

const char*readable_int(char*buf,size_t len,uint64_t val){
	int unit=0;
	const size_t blk=1024;
	const char**units=size_units_ib;
	if(!buf||len<=0)return NULL;
	memset(buf,0,len);
	if(val==0)return strncpy(buf,"0",len-1);
	while((val>=blk)&&units[unit+1])val/=blk,unit++;
	snprintf(buf,len-1,"%"PRIu64" %s",val,units[unit]);
	return buf;
}

const char*readable_float(char*buf,size_t len,uint64_t val){
	int unit=0;
	double v;
	const size_t blk=1024;
	const char**units=size_units_ib;
	if(!buf||len<=0)return NULL;
	memset(buf,0,len);
	if(val==0)return strncpy(buf,"0",len-1);
	while((val>=blk*blk)&&units[unit+1])val/=blk,unit++;
	v=(double)val;
	if(val>blk&&units[unit+1])v/=blk,unit++;
	snprintf(buf,len-1,"%.2lf %s",v,units[unit]);
	return buf;
}

static bool squashfs_check(void*data,size_t len){
	uint32_t val;
	if(len<128)return false;
	memcpy(&val,data,sizeof(val));
	return val==SQUASHFS_MAGIC;
}

static time_t squashfs_get_create_time(void*data,size_t len){
	uint32_t val;
	if(len<128)return false;
	memcpy(&val,data+8,sizeof(val));
	return val;
}

static ssize_t fd_get_size(int fd){
	struct stat st;
	errno=0;
	if(fstat(fd,&st)!=0)return -1;
	if(S_ISREG(st.st_mode))return (ssize_t)st.st_size;
	else if(S_ISBLK(st.st_mode)){
		uint64_t size=0;
		if(ioctl(fd,BLKGETSIZE64,&size)!=0)return -1;
		return (ssize_t)size;
	}else RET_ERRNO(ENOTBLK);
}

static int open_dev_by_partlabel(int flags,const char*label){
	int fd;
	char*dev;
	errno=0;
	if(!(dev=blkid_evaluate_tag("PARTLABEL",label,NULL)))
		WARN_ERRNO(ENODEV,"block device %s not found",label);
	fd=open(dev,flags);
	free(dev);
	return fd;
}

static int block_copy(int dest,int src,size_t len){
	int e=0;
	ssize_t ret;
	size_t offset=0;
	char buff[0x1000];
	errno=0;
	lseek(dest,0,SEEK_SET);
	lseek(src,0,SEEK_SET);
	while(len>offset){
		ret=sendfile(dest,src,NULL,len);
		if(ret==0)return 0;
		if(ret>0||errno==EINTR)continue;
		warn("sendfile call failed");
		lseek(dest,0,SEEK_SET);
		memset(buff,0,sizeof(buff));
		write(dest,buff,sizeof(buff));
		e=errno;
		break;
	}
	readable_float(buff,sizeof(buff),offset);
	printf("transferred %s (%zu bytes)\n",buff,offset);
	lseek(dest,0,SEEK_SET);
	lseek(src,0,SEEK_SET);
	errno=e;
	return e?-1:0;
}

static const char*time_to_string(time_t ts,char*buff,size_t len){
	struct tm*tm;
	const char*fmt="%Y-%m-%d %H:%M:%S";
	if(!buff||len<=0)return NULL;
	memset(buff,0,len);
	if(!(tm=localtime(&ts)))strncpy(buff,"(invalid time)",len-1);
	strftime(buff,len-1,fmt,tm);
	return buff;
}

struct slot_part{
	char name[256];
	size_t size;
	int fd;
};

struct slot_info{
	char name[32];
	time_t time;
	bool squash_valid;
	struct slot_part system;
	struct slot_part boot;
};

static int fill_slot_part(struct slot_part*part,const char*type,const char*slot){
	snprintf(part->name,sizeof(part->name)-1,"%s_%s",type,slot);
	part->fd=open_dev_by_partlabel(O_RDWR,part->name);
	if(part->fd<0)WARN_RET("Open block device %s failed",part->name);
	ssize_t sz=fd_get_size(part->fd);
	if(sz<0)WARN_RET("Get block device %s size failed",part->name);
	part->size=(size_t)sz;
	return 0;
}

static int get_slot_info(struct slot_info*info,const char*slot){
	if(!slot)WARN_ERRNO(EINVAL,"Slots environment variable not set");
	memset(info,0,sizeof(struct slot_info));
	strncpy(info->name,slot,sizeof(info->name)-1);
	if(fill_slot_part(&info->boot,"boot",slot)!=0)return -1;
	if(fill_slot_part(&info->system,"system",slot)!=0)return -1;
	char buff[128];
	if(info->system.size<=sizeof(buff))
		WARN_ERRNO(ENOSPC,"System for slot %s too small",slot);
	ssize_t ret=read(info->system.fd,buff,sizeof(buff));
	if(ret<0)WARN_RET("Read system for slot %s failed",slot);
	if((size_t)ret!=sizeof(buff))WARN_ERRNO(EIO,"Read system for slot %s mismatch",slot);
	info->squash_valid=squashfs_check(buff,sizeof(buff));
	if(info->squash_valid)info->time=squashfs_get_create_time(buff,sizeof(buff));
	return 0;
}

static void print_slot_info(struct slot_info*info,const char*type){
	char buff[64];
	printf("%s slot:\n",type);
	printf("\tSlot name: %s\n",info->name);
	readable_float(buff,sizeof(buff),info->system.size);
	printf("\tSystem size: %s (%zd bytes)\n",buff,info->system.size);
	readable_float(buff,sizeof(buff),info->boot.size);
	printf("\tBoot size: %s (%zd bytes)\n",buff,info->boot.size);
	if(!info->squash_valid)strcpy(buff,"(none)");
	else time_to_string(info->time,buff,sizeof(buff));
	printf("\tSquashFS time: %s\n",buff);
}

static int do_upgrade(void){
	struct slot_info cur,alt;
	if(get_slot_info(&cur,getenv("SLOT_CURRENT"))!=0)
		err(1,"Get current slot info failed");
	if(get_slot_info(&alt,getenv("SLOT_ALTERNATIVE"))!=0)
		err(1,"Get alternative slot info failed");
	print_slot_info(&cur,"Current");
	print_slot_info(&alt,"Alternative");
	if(!cur.squash_valid)
		errx(1,"Current slot have invalid squashfs");
	if(cur.system.size!=alt.system.size)
		errx(1,"System slot size mismatch");
	if(cur.boot.size!=alt.boot.size)
		errx(1,"Boot slot size mismatch");
	if(alt.time==0||cur.time>alt.time){
		puts("Alternative slot upgrade needed");
		puts("Starting copy system...");
		if(block_copy(alt.system.fd,cur.system.fd,cur.system.size)!=0)
			errx(1,"Copy system failed");
		puts("Upgrade alternative system done");
		puts("Starting copy boot...");
		if(block_copy(alt.boot.fd,cur.boot.fd,cur.boot.size)!=0)
			errx(1,"Copy boot failed");
		puts("Upgrade alternative boot done");
		sync();
		puts("Upgrade slots done");
	}else puts("Alternative slot newer or equals to current slot, skip upgrade");
	return 0;
}

int main(int argc,char**argv){
	if(argc!=1||argv[1])
		errx(1,"Usage: slots-upgrade");
	return do_upgrade();
}
