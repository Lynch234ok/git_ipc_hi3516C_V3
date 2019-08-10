#include "system_status.h"

typedef struct CPU_PACKED 
{
	char name[20];    
	unsigned int user;
	unsigned int nice;
	unsigned int system;
	unsigned int idle;
	unsigned int iowait;
}CPU_OCCUPY;

typedef struct MEN_PACKED
{
	char name[20];
	unsigned long total; 
	char name2[20];
	unsigned long free;                       
}MEM_OCCUPY;

static void get_memoccupy (MEM_OCCUPY *mem) 
{
    FILE *fd;          
    int n;             
    char buff[256];   
    MEM_OCCUPY *m;
    m=mem;
                                                                                                              
    fd = fopen ("/proc/meminfo", "r"); 
      
    fgets (buff, sizeof(buff), fd); 
    fgets (buff, sizeof(buff), fd); 
    fgets (buff, sizeof(buff), fd); 
    fgets (buff, sizeof(buff), fd); 
    sscanf (buff, "%s %u %s", m->name, &m->total, m->name2); 
    
    fgets (buff, sizeof(buff), fd); 
    sscanf (buff, "%s %u", m->name2, &m->free, m->name2); 
    
    fclose(fd);   
}

static int cal_cpuoccupy (CPU_OCCUPY *o, CPU_OCCUPY *n) 
{   
    unsigned long od, nd;    
    unsigned long id, sd;
    int cpu_use = 0;   
    
    od = (unsigned long) (o->user + o->nice + o->system +o->idle);
    nd = (unsigned long) (n->user + n->nice + n->system +n->idle);
      
    id = (unsigned long) (n->user - o->user); 
    sd = (unsigned long) (n->system - o->system);
    if((nd-od) != 0)
    cpu_use = (int)((sd+id)*10000)/(nd-od);
    //printf("cpu: %u\n",cpu_use);
    return cpu_use;
}

static int cal_syscpuoccupy (CPU_OCCUPY *o, CPU_OCCUPY *n)
{
    unsigned long od, nd;
    unsigned long sd;
    int cpu_use = 0;

    od = (unsigned long) (o->user + o->nice + o->system + o->idle + o->iowait);
    nd = (unsigned long) (n->user + n->nice + n->system + n->idle + n->iowait);

    sd = (unsigned long) (n->system - o->system);
    if((nd-od) != 0)
    cpu_use = (int)((sd)*10000)/(nd-od);
    //printf("cpu: %u\n",cpu_use);
    return cpu_use;
}

static void get_cpuoccupy (CPU_OCCUPY *cpust) 
{   
    FILE *fd;         
    int n;            
    char buff[256]; 
    CPU_OCCUPY *cpu_occupy;
    cpu_occupy=cpust;
                                                                                                               
    fd = fopen ("/proc/stat", "r"); 
    fgets (buff, sizeof(buff), fd);
    
    sscanf (buff, "%s %u %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice, &cpu_occupy->system, &cpu_occupy->idle, &cpu_occupy->iowait);
    
    fclose(fd);     
}

static CPU_OCCUPY cpu_stat_old = {0};


float cpu_get_status()
{
    CPU_OCCUPY cpu_stat_new;
    MEM_OCCUPY mem_stat;
    float cpu;
    
    //获取内存
    //get_memoccupy ((MEM_OCCUPY *)&mem_stat);
    
    if(strlen(cpu_stat_old.name) == 0){
    	get_cpuoccupy((CPU_OCCUPY *)&cpu_stat_old);
    	//sleep(10);
		return 1.0;
    }
    
    //第二次获取cpu使用情况
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat_new);
    
    //计算cpu使用率
    cpu = (float)cal_cpuoccupy ((CPU_OCCUPY *)&cpu_stat_old, (CPU_OCCUPY *)&cpu_stat_new)/100;


	printf("cpu status: %4.2f\r\n", cpu);
    return cpu;
} 


static CPU_OCCUPY sys_cpu_stat_old = {0};

float cpu_sys_get_status()
{
    CPU_OCCUPY cpu_stat_new;
    MEM_OCCUPY mem_stat;
    float cpu;

    if(strlen(sys_cpu_stat_old.name) == 0)
    {
        get_cpuoccupy((CPU_OCCUPY *)&sys_cpu_stat_old);
        return 1.0;
    }

    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat_new);

    cpu = (float)cal_syscpuoccupy ((CPU_OCCUPY *)&sys_cpu_stat_old, (CPU_OCCUPY *)&cpu_stat_new)/100;

    //printf("sys cpu status: %4.2f\r\n", cpu);
    return cpu;
}

typedef struct tagCpuInfo_
{
    unsigned char cpuName[8];
    unsigned int user;
    unsigned int sys;
    unsigned int nice;
    unsigned int idel;
    unsigned int iowait;
    unsigned int irq;
    unsigned int softirq;
}stCpuInfo, *pstCpuInfo;

static int sys_cpuInfoGet(pstCpuInfo pCpuInfo)
{
    FILE *fp = NULL;
    char buf[128] = {0};
    unsigned int cpu1 = 0, cpu2 = 0;

    fp = fopen("/proc/stat", "r");
    if(NULL == fp)
    {
        printf("open /proc/stat failed!!!\n");
        return -1;
    }

    fgets(buf, sizeof(buf), fp);

    memset(pCpuInfo, 0, sizeof(stCpuInfo));
    sscanf(buf, "%s%d%d%d%d%d%d%d", pCpuInfo->cpuName, &pCpuInfo->user, &pCpuInfo->nice, &pCpuInfo->sys, &pCpuInfo->idel, &pCpuInfo->iowait, &pCpuInfo->irq, &pCpuInfo->softirq);

    fclose(fp);

}

float sys_cpuUsage()
{
    static stCpuInfo old_cpuInfo;
    stCpuInfo new_cpuInfo;

    sys_cpuInfoGet(&new_cpuInfo);

    unsigned long cpuUsage1 = old_cpuInfo.user + old_cpuInfo.sys + old_cpuInfo.nice + old_cpuInfo.idel + old_cpuInfo.iowait + old_cpuInfo.irq + old_cpuInfo.softirq;
    unsigned long cpuUsage2 = new_cpuInfo.user + new_cpuInfo.sys + new_cpuInfo.nice + new_cpuInfo.idel + new_cpuInfo.iowait + new_cpuInfo.irq + new_cpuInfo.softirq;
    unsigned long total = cpuUsage2 - cpuUsage1;

    float cpuUsage = (float)(new_cpuInfo.sys - old_cpuInfo.sys) / total * 100;
    //printf("\n");
    //printf("====> cpuUsage1 = %d\n", new_cpuInfo.sys);
    //printf("====> cpuUsage2 = %d\n", old_cpuInfo.sys);
    //printf("===> cpuUsage2 - cpuUsage1 = %ld\n", cpuUsage2 - cpuUsage1);
    //printf("====> cpuUsage = %.2f\n", cpuUsage);

    old_cpuInfo.user = new_cpuInfo.user;
    old_cpuInfo.sys = new_cpuInfo.sys;
    old_cpuInfo.nice = new_cpuInfo.nice;
    old_cpuInfo.idel = new_cpuInfo.idel;
    old_cpuInfo.iowait = new_cpuInfo.iowait;
    old_cpuInfo.irq = new_cpuInfo.irq;
    old_cpuInfo.softirq = new_cpuInfo.softirq;

    return cpuUsage;

}

