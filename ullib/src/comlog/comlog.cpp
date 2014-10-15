/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: comlog.cpp,v 1.5.6.9 2010/04/26 13:36:04 zhang_rui Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file log.cpp
 * @author xiaowei(com@baidu.com)
 * @date 2008/03/25 11:26:52
 * @version $Revision: 1.5.6.9 $ 
 * @brief 
 *  
**/

#include <pthread.h>
#include <ul_conf.h>
#include "ul_log.h"
#include "comlog.h"
#include "logcore.h"
#include "category.h"
#include "appender/appender.h"
#include "appender/appenderfactory.h"
#include "layout.h"
#include "xutils.h"
#include "comsig.h"
#include "namemg.h"
#include "logswitcher.h"

void ul_set_log_level(int lv);/** ����ȫ����־�ȼ�  */
static int g_comlog_global_level = 16;

static comspace::Category *g_main_cg = NULL;
#define _ROOT "MAIN"


/** ����ʽ����com_openlog��com_loadlogʱ��ȫ�ֱ���������Ϊ1������ʹ��comlog��ӡ��־
 * δ��ʽ���û���õ���ul_log�����Ƶĺ���������ʹ��ullog��ӡ��־
 */
namespace comspace{
static int __log_switcher__ = comspace :: __LOG_NOT_DEFINED__;	//���ڽ��ul_log��com_log��������õ�ȫ�ֱ���
int com_get_log_switcher() {
	return __log_switcher__;
}
void com_set_log_switcher(int swt){
	__log_switcher__ = swt;
}
};

static comspace :: AppenderFactory __appender_factory; //ֻΪ�˳�ʼ��AppenderFactory����

namespace comspace{
class xAtomic{
public:
	pthread_mutex_t _lock;
	int _cnt;
public:
	xAtomic(){
		pthread_mutex_init(&_lock, NULL);
		_cnt = 0;
	}
	void add(){
		//_com_error("xAtomic::Add  %u", pthread_self());
		xAutoLock __lock(&_lock);
		++_cnt;
	}
	void sub(){
		//_com_error("xAtomic::Sub  %u", pthread_self());
		xAutoLock __lock(&_lock);
		--_cnt;
	}
	int cnt(){
		xAutoLock __lock(&_lock);
		return _cnt;
	}
	void reset(int n){
		xAutoLock __lock(&_lock);
		_cnt = n;
	}
};

}
static comspace::xAtomic g_close_atomic;
static comspace::xAtomic g_closing;


static const int g_loglvlary[] = 
{
	COMLOG_FATAL,
	COMLOG_WARNING,
	COMLOG_NOTICE,
	COMLOG_TRACE,
	COMLOG_DEBUG,
};

static char g_proc_name[256] = "Default";
static pthread_key_t g_comlog_key = PTHREAD_KEYS_MAX;
static pthread_once_t g_comlog_ronce = PTHREAD_ONCE_INIT;
pthread_key_t* comlog_key(){
	return &g_comlog_key; //used by logcore.cpp close_log
}

static void comlog_destroy(void * ptr)
{
	if(ptr){
		delete (com_thread_data_t *)ptr;
	}
}
static void comlog_runonce()
{
	pthread_key_create(&g_comlog_key, comlog_destroy);
}

//�Ƿ��Ѿ�����openlog_r
static pthread_key_t key_open_r = PTHREAD_KEYS_MAX;

static pthread_once_t once_open_r = PTHREAD_ONCE_INIT;
static void key_destructor(void *){}
static void com_open_r_once(){
	pthread_key_create(&key_open_r, key_destructor);
}

/**
 * @brief  ��ò�����ָ���ȼ���COMLOG_LEVEL����ϵͳ��־
 *         �ȼ���logstat.sysevents��
 *
 *         ullib_3-1-47-1����һ��Bug��ʹ��com_changelogstat֮��
 *         ��־�ȼ������趨Ϊָ���ȼ�����com_openlog�����趨Ϊ��
 *         ����ָ���ȼ����������������ı�������ͳһ���߼���
 *
 *         ����������comlog_get_lower_syslevel(4)��������
 *               (1 << 4) | (1 << 2) | (1 << 1) = 0x16
 * @note   �ú�����Ҫ��¶��comsig.cpp������û������Ϊstatic��
 *         ��Ҳ����Ҫ��¶���û�������û����comlog.h��������
 * @author zhutianchi
 * @date   2011/09/27 07:35:41
**/
namespace comspace {
u_int comlog_get_lower_syslevel(int sysevents)
{
	u_int syslvl = 0;
	int size = sizeof g_loglvlary / sizeof g_loglvlary[0];
	for (int i = 0; i < size; ++i) {
		if (sysevents >= g_loglvlary[i]) {
			syslvl |= ((u_int)1 << g_loglvlary[i]);
		}
	}
	return syslvl;
}
}

static com_thread_data_t *comlog_getdata()
{
	pthread_once(&g_comlog_ronce, comlog_runonce);
	com_thread_data_t *ptr = (com_thread_data_t *)pthread_getspecific(g_comlog_key);
	if (ptr == NULL) {
		ptr = new com_thread_data_t;
		ptr->category = NULL;
		ptr->event = comspace::Event::getEvent();
		snprintf(ptr->event->_proc_name, sizeof(ptr->event->_proc_name), "%s", g_proc_name);
		pthread_setspecific(g_comlog_key, ptr);
	}
	return ptr;
}

int com_openlog(const char *procname, com_device_t *dev, int dev_num, com_logstat_t *stat)
{
	if (comspace::com_get_log_switcher() != comspace::__LOG_NOT_DEFINED__){
		_com_error("Log is open....");
		return -1;
	}
	if (dev == NULL) {
		return -1;
	}
	if (dev_num < 0) {
		return -1;
	}
	snprintf(g_proc_name, sizeof(g_proc_name), "%s", procname);

	int ret = comspace::init_log();
	comspace::Appender *app = NULL;
	com_thread_data_t *ptr = NULL;

	if (ret != 0) {
		_com_error("init log err");
		return -1;
	}
	g_main_cg = comspace::Category::getInstance(_ROOT);

	if (g_main_cg == NULL) {
		_com_error("get %s category err", _ROOT);
		goto fail;
	}

	//category��event
	ptr = comlog_getdata();
	ptr->category = g_main_cg;
	g_main_cg->_event = ptr->event;
	g_main_cg->setLogStat(comspace::comlog_get_def_logstat());
	
	if (stat != NULL) {
		ul_set_log_level(stat->sysevents);
		comlog_set_log_level(stat->sysevents);

		u_int syslvl = comspace::comlog_get_lower_syslevel(stat->sysevents);
		if (stat->userevents[0] == 0) {
			comspace::comlog_get_def_logstat()->setLevel(syslvl, NULL);
		} else {
			comspace::comlog_get_def_logstat()->setLevel(syslvl, stat->userevents);
		}
	}

	if (0 == dev_num) {
		com_device_t devtty;
		strncpy(devtty.type, "TTY", COM_MAXAPPENDERNAME);
		COMLOG_SETSYSLOG(devtty);
		app = comspace::AppenderFactory::getAppender(devtty);
		if (app == NULL) {
			_com_error("create appender default TTY err");
			goto fail;
		}
		g_main_cg->addAppender(app);
	}
	for (int i=0; i<dev_num; ++i) {
		app = comspace::AppenderFactory::getAppender(dev[i]);
		if (app == NULL) {
			_com_error("create appender %s:%s err", dev[i].host, dev[i].file);
			goto fail;
		}
		g_main_cg->addAppender(app);
	}

//	comspace::__log_switcher__ = comspace::__USING_COMLOG__;
	com_set_log_switcher(comspace::__USING_COMLOG__);

	g_close_atomic.reset(0);//�̼߳���������
	g_close_atomic.add();

	com_writelog(COMLOG_NOTICE, "--------------------------------------------------------------- open log ------------");
	com_writelog(COMLOG_WARNING, "--------------------------------------------------------------- open log.wf ------------");
	return 0;
fail:
	int log_switcher = comspace::com_get_log_switcher();
//	comspace::__log_switcher__ = comspace::__USING_COMLOG__;
	com_set_log_switcher(comspace::__USING_COMLOG__);
	com_closelog(1000);
//	comspace::close_log();
//	comspace::__log_switcher__ = log_switcher;
	comspace::com_set_log_switcher(log_switcher);
	return -1;
}

int com_resetlog()
{
	int ret = 0;

	if (comspace::com_get_log_switcher() != comspace::__USING_COMLOG__) {
		_com_debug("Nothing need to reset");
		return 0;
	}

	/* The appender need implement its own reset function */
	ret = comspace::com_reset_appender();

	/* Destory nameMg etc. */
	comspace::reset_log();

	/* After fork(2) pthread_self() will return the same id as his parent,
	 * and pthread_once() will not run again, so clear the pthread_key_t 
	 * in multprocess.
	 */
	pthread_setspecific(g_comlog_key, NULL);
	pthread_key_delete(g_comlog_key);
	comlog_runonce();
	pthread_setspecific(key_open_r, NULL);
	pthread_key_delete(key_open_r);
	com_open_r_once();
	comspace::Event::reset_event_key();

	com_set_log_switcher(comspace::__LOG_NOT_DEFINED__);
	g_closing.reset(0);
	g_close_atomic.reset(0);

	return ret;
}


int com_closelog(int waittime)
{
	if(comspace::com_get_log_switcher() != comspace::__USING_COMLOG__){
		//comspace::comlog_del_def_layout();
		//comspace::comlog_del_nameMg();
		//comspace::SendSvr::comlog_del_gcheckSvr();
		return ul_closelog(waittime);
	}
	//close������ʮ�ָ��ӣ�(yingxiang@baidu.com, 20081223)
	//��1���ȴ��첽�̷߳�����ϣ���ֹ��־��ʧ
	g_closing.reset(1);//��ֹ��ʱ�����µ��߳� openlog_r
	comspace::com_wait_close(waittime);//�ȴ��첽�̴߳�ӡ��־,NetAppender/AsyncFileAppender...
	g_close_atomic.sub();//���̵߳ļ�����-1�����̵߳��������������ġ�

	//��2��Appenderֹͣ���������ݡ��첽��ӡ��Appender��ʱӦ��ֹ�Լ����̡߳�
	//��Ϊ����첽�̱߳���Ҳ�ܻ����writelog��������Ҫclose֮������ռ�������
	//FileAppender�������Ӵ˲�������Ϊ��û���첽�̡߳������ڵڣ�4�������˳���
	comspace::com_close_appender();
	//��3������̼߳������Ƿ�Ϊ0���ȴ�10����ǿ�ƹر�
	//�����������Ϊ0����ʾ�������߳̿������ڴ�ӡ��־����core��
	//���Գ���Ա�ڶ��̴߳�ӡ��ʱ��Ӧ��֤ÿ���̶߳���closelog_r������closelog��Σ�յ�
	int cnt = 10;
	while(g_close_atomic.cnt() > 0 && cnt--){
		//��ʱAppender�Ѿ�ֹͣ�������ݣ����Բ�������comlog��ӡ��־
		//com_writelog(COMLOG_NOTICE, "Closelog: waiting for other thread......");
		_com_error("Closelog: Waiting for other thread......");
		comspace::safe_msleep(1000);
	}
	if((cnt = g_close_atomic.cnt()) > 0){
		while(cnt--){
			//com_writelog(COMLOG_FATAL, "Did you forget to call com_closelog_r in some threads?.....");
			_com_error("Did you forget to call com_closelog_r in some threads?.....");
		}
	}
	//��4��������ݣ���β������	
	int ret = comspace::close_log(waittime);
//	comspace::__log_switcher__ = comspace::__LOG_NOT_DEFINED__;
	com_set_log_switcher(comspace::__LOG_NOT_DEFINED__);
	g_closing.reset(0);
//	comspace::comlog_del_def_layout();
//	comspace::comlog_del_nameMg();
//	comspace::SendSvr::comlog_del_gcheckSvr();
	return ret;
}

int com_openlog_r()
{
	//δ��ʽ����com_openlog()����һ��ת�Ƶ�ul_log
	if (comspace::com_get_log_switcher() != comspace::__USING_COMLOG__) {
		ul_logstat_t logstat; 
		logstat.events = ul_get_log_level();
		logstat.to_syslog = UL_LOG_FATAL;
		logstat.spec = UL_LOGTTY;
		return ul_openlog_r(NULL, &logstat, NULL);
	}
	//_com_error("Gonna openlog_r %u", pthread_self());
	pthread_once(&once_open_r, com_open_r_once);
	if(pthread_getspecific(key_open_r) != NULL){
		//already open_r
		return -1;
	}
	comspace::xAutoLock __lock(&(g_closing._lock));
	if(g_closing._cnt == 1){
		return -1;
	}
	//int ret = comspace::init_log_r();
	pthread_t tid = pthread_self();
	char name[256];
	snprintf(name, sizeof(name), "%s.%lu", _ROOT, (unsigned long)tid);
	comspace::Category *cate = comspace::Category::getInstance(name);
	if (cate == NULL) {
		_com_error("what's wrong, cate[%s] create err", name);
		return -1;
	}
	com_thread_data_t *ptr = comlog_getdata();
	ptr->category = cate;
	cate->_event = ptr->event;

	//���g_main_cg�ж�openlog�Ƿ񱻵��ù�
	//���appender���ж��Ƿ�û��appender
	//����Ĭ�ϵ�ttyappender
	if (g_main_cg == NULL && cate->appender_num() <= 0) {
		_com_debug("add default appender");
		com_device_t dev;
		snprintf(dev.type, COM_MAXAPPENDERNAME, "%s", "TTY");
		COMLOG_SETSYSLOG(dev);
		comspace::Appender *app = comspace::AppenderFactory::getAppender(dev);
		if (app != NULL) {
			ptr->category->addAppender(app);
		}
	}

	com_writelog(COMLOG_NOTICE, "--------------------------------------------------------------- open log_r.%u ------------", (unsigned long)tid);
	com_writelog(COMLOG_WARNING, "--------------------------------------------------------------- open log_r.%u.wf ------------", (unsigned long)tid);
	g_close_atomic.add();
	pthread_setspecific(key_open_r, (void *)pthread_self());
	//_com_error("Finished openlog_r %u", pthread_self());
	return 0;
}

int com_closelog_r()
{
	//δ��ʽ����com_openlog��һ��ת�Ƶ�ul_log
	if(comspace::com_get_log_switcher() != comspace::__USING_COMLOG__) {
		return ul_closelog_r(0);
	}

	//_com_error("Gonna closelog_r %u", pthread_self());
	pthread_once(&once_open_r, com_open_r_once);
	if(pthread_getspecific(key_open_r) == NULL){
		//not open or already closed.
		return -1;
	}
	pthread_t tid = pthread_self();
	com_writelog(COMLOG_NOTICE, "--------------------------------------------------------------- close log_r.%u ------------", (unsigned long)tid);
	com_writelog(COMLOG_WARNING, "--------------------------------------------------------------- close log_r.%u.wf ------------", (unsigned long)tid);
	g_close_atomic.sub();
	pthread_setspecific(key_open_r, NULL);
	//_com_error("Finished closelog_r %u", pthread_self());
	return 0;
}

static com_thread_data_t * gethandle()
{

	if(comspace::com_get_log_switcher() == comspace :: __LOG_NOT_DEFINED__) {
//		comspace::__log_switcher__ = comspace :: __USING_COMLOG__;
		com_set_log_switcher(comspace :: __USING_COMLOG__);
	}

	com_thread_data_t *data = comlog_getdata();
	if (data->category == NULL) {
		//_com_debug("get null category need creat");
		//�Ƿ��ʼ������־
		comspace::init_log();
		if (com_openlog_r() != 0) {
			_com_error("com_openlog_r default err");
			return NULL;
		}
		//g_close_atomic.sub();
	}
	return data;
}

extern int ul_writelog_args(const int event, const char* fmt, va_list args);

int com_setappendermask(char * appender_type) 
{
        int ret;

	if (appender_type == NULL)
		return 0;

        //δ��ʽ����com_openlog()��ֱ�ӷ���
        if(comspace::com_get_log_switcher() != comspace :: __USING_COMLOG__) {
                _com_error("com_openlog not called!");
                return -1;
        }
        
        com_thread_data_t *ptr = gethandle();
        if (ptr == NULL) {
                _com_error("gethandle error");
                return -1;
        }
        
        ret = ptr->category->addAppenderMask(appender_type);
                
        if( ret == -1 )
                _com_error("can not find appender type : %s",appender_type );

        return ret;
}   

int com_writelog(int events, const char *fmt, ...)
{
	
	va_list args;
	va_start(args, fmt);

	int ret;
	//δ��ʽ����com_openlog()��һ��ת�Ƶ�ul_log
	if(comspace::com_get_log_switcher() != comspace :: __USING_COMLOG__) {
		ret = ul_writelog_args(events, fmt, args);
		va_end(args);
		return ret;
	}

	com_thread_data_t *ptr = gethandle();
	if (ptr == NULL) {
		_com_error("gethandle error");
		return -1;
	}

	ret = ptr->category->print(events, fmt, args);
	va_end(args);

	return ret;
}

int com_writelog_ex(int events, int version, const char *type,
	const char *fmt, ...)
{
	
	va_list args;
	va_start(args, fmt);

	int ret;
	//δ��ʽ����com_openlog()��һ��ת�Ƶ�ul_log
	if(comspace::com_get_log_switcher() != comspace :: __USING_COMLOG__) {
		ret = ul_writelog_args(events, fmt, args);
		va_end(args);
		return ret;
	}

	com_thread_data_t *ptr = gethandle();
	if (ptr == NULL) {
		_com_error("gethandle error");
		return -1;
	}

	if (version < 0) {
		_com_error("log version=%d should >= 0", version);
		return -1;
	} else if (*type == 0) {
		_com_error("log type should not be NULL");
		return -1;
	} else {
		ptr->event->setLogVersion(version);
		ptr->event->setLogType(type);
	}

	ret = ptr->category->print(events, fmt, args);
	va_end(args);

	return ret;
}

int com_writelog_args(int events, const char *fmt, va_list args)
{
	int ret;
	//δ��ʽ����com_openlog()��һ��ת�Ƶ�ul_log
	if(comspace::com_get_log_switcher() != comspace :: __USING_COMLOG__) {
		ret = ul_writelog_args(events, fmt, args);
		return ret;
	}

	com_thread_data_t *ptr = gethandle();
	if (ptr == NULL) {
		_com_error("gethandle error");
		return -1;
	}

	ret = ptr->category->print(events, fmt, args);

	return ret;
}

int com_writelog(const char *name, const char *fmt, ...)
{
	com_thread_data_t *ptr = gethandle();
	if (ptr == NULL) {
		_com_error("gethandler error");
		return -1;
	}
	
	u_int id = comspace::getLogId(name);
	if (id == 0) {
		_com_error("get logiderr[%s]", name);
		return -1;
	}

	va_list args;
	va_start(args, fmt);
	int ret = ptr->category->print(id, fmt, args);
	va_end(args);

	return ret;
}

int com_writelog_ex(const char *name, int version, const char *type,
	const char *fmt, ...)
{
	com_thread_data_t *ptr = gethandle();
	if (ptr == NULL) {
		_com_error("gethandler error");
		return -1;
	}
	
	u_int id = comspace::getLogId(name);
	if (id == 0) {
		_com_error("get logiderr[%s]", name);
		return -1;
	}

	if (version < 0) {
		_com_error("log version=%d should >= 0", version);
		return -1;
	} else if (*type == 0) {
		_com_error("log type should not be NULL");
		return -1;
	} else {
		ptr->event->setLogVersion(version);
		ptr->event->setLogType(type);
	}

	va_list args;
	va_start(args, fmt);
	int ret = ptr->category->print(id, fmt, args);
	va_end(args);

	return ret;
}

com_thread_data_t *com_getthreaddata()
{
	return gethandle();
}

comspace::Category * com_getcategory()
{
	if (comspace::com_get_log_switcher() == comspace::__USING_ULLOG__){
		_com_error("[com_getcategory]Error: Using ullog now....");
		return NULL;
	}
	return gethandle()->category;
}

comspace::Event * com_getevent()
{
	return gethandle()->event;
}

int com_loadlog(const char *path, const char *file)
{
	return com_loadlog_ex(path, file, 0);
}

int com_loadlog_ex(const char *path, const char *file, int pid)
{
	Ul_confdata *ptr = ul_initconf(MAX_CONF_ITEM_NUM-1);
	if (ptr == NULL) {
		_com_error("init conf err");
		return -1;
	}

	int ret = ul_readconf(path, file, ptr);
	if (ret != 1) {
		ul_freeconf(ptr);
		_com_error("read conf err");
		return -1;
	}

	char flag[256];
	char name[256];
	char val[1024*2];
	char procname[256];
	char timefmt[128];
	com_device_t *devices = NULL;
	com_device_t nuldev;

	com_logstat_t logstat;
	memset(&logstat, 0, sizeof(logstat));


	//��ȡģ����
	snprintf(flag, sizeof(flag), "%s", COMLOG_PROCNAME);
	ret = ul_getconfnstr(ptr, flag, procname, sizeof(procname));
	procname[sizeof(procname)-1] = 0;
	if (ret != 1) {
		snprintf(procname, sizeof(procname), "NoModuleName");
	}

	snprintf(flag, sizeof(flag), "%s", COMLOG_LEVEL);
	ret = ul_getconfint(ptr, flag, &logstat.sysevents);
	if (ret != 1) {
		logstat.sysevents = 16;
	}
	//ת���Զ���ȼ�
	snprintf(flag, sizeof(flag), "%s", COMLOG_SELFDEFINE);
	ret = ul_getconfnstr(ptr, flag, logstat.userevents, sizeof(logstat.userevents));
	logstat.userevents[sizeof(logstat.userevents)-1] = 0;
	if (ret != 1) {
		logstat.userevents[0] = 0;
	}

	snprintf(flag, sizeof(flag), "%s", COMLOG_TIME_FORMAT);
	ret = ul_getconfnstr(ptr, flag, timefmt, sizeof(timefmt));
	timefmt[sizeof(timefmt)-1] = 0;
	if(ret == 1){//����û��
		comspace::Layout::setTimeFormat(timefmt);
	}

	snprintf(flag, sizeof(flag), "%s", COMLOG_LOGLENGTH);
	int loglength = 2048;
	ret = ul_getconfint(ptr, flag, &loglength);
	if(ret == 1){
		comspace::Event::setMaxLogLength(loglength);
	}

	int num = 0;
	int tmp = 0;
	snprintf(flag, sizeof(flag), "%s", COMLOG_DEVICE_NUM);
	ret = ul_getconfint(ptr, flag, &num);
	
	if (num > 0) {
		devices = new com_device_t[num];

		for (int i=0; i<num; ++i) {
			snprintf(flag, sizeof(flag), "%s%d", COMLOG_DEVICE, i);
			devices[i].reserved_num = 0;
			ret = ul_getconfnstr(ptr, flag, name, sizeof(name));
			if (ret != 1) {
				_com_error("can't get flag[%s]", flag);
				ret = -1;
				goto end;
			}

			//type
			snprintf(flag, sizeof(flag), "%s_%s", name, COMLOG_DEVICE_TYPE);
			ret = ul_getconfnstr(ptr, flag, val, sizeof(val));
			if (ret != 1) {
				_com_error("can't get flag[%s]", flag);
				ret = -1;
				goto end;
			}
			if(comspace::AppenderFactory::isAppender(val)){
				strcpy(devices[i].type, val);
			}
			else{
				_com_error("Unkown device type[%s] at flag[%s]", val, flag);
				ret = -1;
				goto end;
			}

#if 0
			if (strcmp(val, COMLOG_DEVICE_FILE) == 0) {
				devices[i].type = COM_FILEDEVICE;
			} else if (strcmp(val, COMLOG_DEVICE_NET) == 0) {
				devices[i].type = COM_NETDEVICE;
			} else if (strcmp(val, COMLOG_DEVICE_TTY) == 0) {
				devices[i].type = COM_TTYDEVICE;
			} else if (strcmp(val, COMLOG_DEVICE_ULLOG) == 0) {
				devices[i].type = COM_ULLOGDEVICE;
			} else if (strcmp(val, COMLOG_DEVICE_ULNET) == 0) {
				devices[i].type = COM_ULNETDEVICE;
			} else if (strcmp(val, COMLOG_DEVICE_AFILE) == 0) {
				devices[i].type = COM_AFILEDEVICE;			
			}else {
				_com_error("invalid conf[%s] at flag[%s]", val, flag);
				ret = -1;
				goto end;
			}
#endif
			//path
			snprintf(flag, sizeof(flag), "%s_%s", name, COMLOG_DEVICE_PATH);
			ret = ul_getconfnstr(ptr, flag, devices[i].host, sizeof(devices[i].host));
			if (ret != 1 && strcmp(devices[i].type, "TTY") != 0) {
				_com_error("can't get flag[%s]", flag);
				ret = -1;
				goto end;
			}
			//name
			snprintf(flag, sizeof(flag), "%s_%s", name, COMLOG_DEVICE_NAME);
			ret = ul_getconfnstr(ptr, flag, devices[i].file, sizeof(devices[i].file));
			if (ret != 1 && strcmp(devices[i].type,"TTY") != 0) {
				_com_error("can't get flag[%s]", flag);
				ret = -1;
				goto end;
			}
			
			//reserved-----------------------------------------------
			snprintf(flag, sizeof(flag), "%s_%s", name, COMLOG_DEVICE_RESERVED1);
			ret = ul_getconfnstr(ptr, flag, devices[i].reserved1, COM_RESERVEDSIZE);
			snprintf(flag, sizeof(flag), "%s_%s", name, COMLOG_DEVICE_RESERVED2);
			ret = ul_getconfnstr(ptr, flag, devices[i].reserved2, COM_RESERVEDSIZE);
			snprintf(flag, sizeof(flag), "%s_%s", name, COMLOG_DEVICE_RESERVED3);
			ret = ul_getconfnstr(ptr, flag, devices[i].reserved3, COM_RESERVEDSIZE);
			snprintf(flag, sizeof(flag), "%s_%s", name, COMLOG_DEVICE_RESERVED4);
			ret = ul_getconfnstr(ptr, flag, devices[i].reserved4, COM_RESERVEDSIZE);
			//reserved����չ���飬ΪӦ����������������
			//�ⲿ���Լ���������ul_conf�Ŀ�, ���� name _ xxxx�����ö�����,����path�������ֶ�

			char name_pre[sizeof(name)];
			int name_pre_len = snprintf(name_pre, sizeof(name_pre), "%s_", name);
			for (int j = 0; j < ptr->num; ++j) {
				if (strncmp((ptr->item[j]).name, name_pre, name_pre_len) == 0 
						|| strncmp((ptr->item[j]).name, COMLOG_DFS, sizeof(COMLOG_DFS)-1) == 0) { 
					snprintf(devices[i].reservedext[devices[i].reserved_num].name,
							sizeof(devices[i].reservedext[0].name),
							"%s",   
							(ptr->item[j]).name);
					snprintf(devices[i].reservedext[devices[i].reserved_num].value,
							sizeof(devices[i].reservedext[0].value),
							"%s",   
							(ptr->item[j]).value);
					devices[i].reserved_num ++;
					if (devices[i].reserved_num >= COM_RESERVEDNUM) {
						break;
					}
				}
			}

			/* For bigpipe appender on multiprocess, pid for
			 * mkdir default disklog path
			 * by wangshaoyan@baidu.com
			 */
			if (pid < 0) {
				_com_error("Illegal para pid = %d", pid);
				ret = -1;
				goto end;
			} else if (!strncmp(devices[i].type, "BIGPIPE", sizeof(devices[i].type))
				&& devices[i].reserved_num < COM_RESERVEDNUM) {
				snprintf(devices[i].reservedext[devices[i].reserved_num].name,
					sizeof(devices[i].reservedext[0].name),
					"%s_%s", name, "DISKLOG_PATH_ID");
				snprintf(devices[i].reservedext[devices[i].reserved_num].value,
					sizeof(devices[i].reservedext[0].value), "%d", pid);
				devices[i].reserved_num++;
			}
			snprintf(devices[i].name, sizeof(devices[i].name), "%s", name);
			devices[i].set_reservedconf();

			//open
			snprintf(flag, sizeof(flag), "%s_%s", name, COMLOG_DEVICE_OPEN);
			ret = ul_getconfint(ptr, flag, &tmp);
			if (ret == 1) {
				devices[i].open = tmp;
			}

			//splite_type
			snprintf(flag, sizeof(flag), "%s_%s", name, COMLOG_DEVICE_SPLITE_TYPE);
			char sp_buf[32];
			ret = ul_getconfnstr(ptr, flag, sp_buf, 31);
			if (ret == 1) {
				if(strcmp(sp_buf, COMLOG_DEVICE_TRUNCT) == 0){
					devices[i].splite_type = COM_TRUNCT;
				}
				else if(strcmp(sp_buf, COMLOG_DEVICE_SIZECUT) == 0){
					devices[i].splite_type = COM_SIZECUT;
				}
				else if(strcmp(sp_buf, COMLOG_DEVICE_DATECUT) == 0){
					devices[i].splite_type = COM_DATECUT;
				}
				else{
					if( ul_getconfint(ptr, flag, &tmp) == 1 ){
						if(tmp != COM_TRUNCT && tmp != COM_SIZECUT && tmp != COM_DATECUT){
							_com_error("Error config for comlog: %s : %s", flag, sp_buf);
						}
						else{
							devices[i].splite_type = tmp;
						}
					}
					else{
						_com_error("Error config for comlog: %s : %s", flag, sp_buf);
					}
				}
			}

			//��־��С
			snprintf(flag, sizeof(flag), "%s_%s", name, COMLOG_DEVICE_SIZE);
			ret = ul_getconfint(ptr, flag, &tmp);
			if (ret == 1) {
				devices[i].log_size = tmp;
			}

			//compress
			snprintf(flag, sizeof(flag), "%s_%s", name, COMLOG_DEVICE_COMPRESS);
			ret = ul_getconfint(ptr, flag, &tmp);
			if (ret == 1) {
				devices[i].compress = tmp;
			}

			//cuttime ��־�и�����
			snprintf(flag, sizeof(flag), "%s_%s", name, COMLOG_DEVICE_DATECUTTIME);
			ret = ul_getconfint(ptr, flag, &tmp);
			if (ret == 1) {
				if (tmp > 0) {
					devices[i].cuttime = tmp;
				}
			}

			//�Ƿ�֧��ϵͳ��־
			snprintf(flag, sizeof(flag), "%s_%s", name, COMLOG_DEVICE_SYSLEVEL);
			ret = ul_getconfint(ptr, flag, &tmp);
			if (ret != 1) tmp = 16;
			if (tmp < 32) {
				devices[i].log_mask = 0;
				for (int j=1; j<=tmp; ++j) {
					devices[i].log_mask |= (1<<j);
				}
			} else {
				_com_error("invalid %s = %d", name, tmp);
			}

			//֧�ֵ��Զ�����־�ȼ�
			snprintf(flag, sizeof(flag), "%s_%s", name, COMLOG_DEVICE_SELFLEVEL);
			ret = ul_getconfnstr(ptr, flag, val, sizeof(val)-1);
			val[sizeof(val)-1] = 0;
			if (ret == 1) {
				set_comdevice_selflog(devices+i, val);
			}

			//��Ȩ
			snprintf(flag, sizeof(flag), "%s_%s", name, COMLOG_DEVICE_AUTH);
			ret = ul_getconfnstr(ptr, flag, devices[i].auth, sizeof(devices[i].auth));
			if (ret != 1) {
				devices[i].auth[0] = 0;
			}

			//layout
			snprintf(flag, sizeof(flag), "%s_%s", name, COMLOG_DEVICE_LAYOUT);
			comspace::Layout *layout = comspace::comlog_get_def_layout();
			ret = ul_getconfnstr(ptr, flag, val, sizeof(val));

			_com_debug("getpattern:at device[%d]--  %s:%s", i, flag, val);
			if (ret == 1) {
				layout = new comspace::Layout();
				_com_debug("new Layout for %s", flag);
				layout->setPattern(val);
			}
			snprintf(flag, sizeof(flag), "%s_%s", name, COMLOG_DEVICE_LAYOUT_NDC);
			ret = ul_getconfnstr(ptr, flag, val, sizeof(val)-1);
			if (ret == 1) {
				if (layout == comspace::comlog_get_def_layout()) {
					layout = new comspace::Layout();
					_com_debug("new layout for %s", flag);
				}
				layout->setNdcPattern(val);
			}
			devices[i].layout = layout;
			_core_debug("def=%lx now=%lx", comspace::comlog_get_def_layout(), layout);
		}
	} else {
		devices = &nuldev;
	}
	ret = com_openlog(procname, devices, num, &logstat);
	if (devices == &nuldev) {
		devices = NULL;
	}
end:
	if (devices) {
		delete [] devices;
	}
	ul_freeconf(ptr);
	return ret;
}

com_device_t::com_device_t()
{
	set_comdevice_default(this);
}

int set_comdevice_default(com_device_t *dev)
{
	if (dev == NULL) return -1;
	dev->host[0] = 0;
	dev->file[0] = 0;
	dev->auth[0] = 0;
	dev->type[0] = 0;
	dev->reserved1[0] = 0;
	dev->reserved2[0] = 0;
	dev->reserved3[0] = 0;
	dev->reserved4[0] = 0;
	dev->log_size = 2048;
	dev->open = 1;
	dev->splite_type = COM_TRUNCT;
	dev->compress = 0;
	dev->log_mask = 0;
	dev->cuttime = 60;
	dev->layout = comspace::comlog_get_def_layout();
	dev->reserved_num = 0;
	dev->reservedconf.num = 0;
#if 0
	COMLOG_ADDMASK(*dev, COMLOG_FATAL);
	COMLOG_ADDMASK(*dev, COMLOG_WARNING);
	COMLOG_ADDMASK(*dev, COMLOG_NOTICE);
	COMLOG_ADDMASK(*dev, COMLOG_TRACE);
	COMLOG_ADDMASK(*dev, COMLOG_DEBUG);
#endif
	return 0;
}

void com_device_t::set_reservedconf()
{
	reservedconf.item = reservedext;
	reservedconf.num  = reserved_num;
	reservedconf.size = reserved_num;
}

int com_setdevices(com_device_t *dev, int nums, char *selflog)
{
	if (dev == NULL || nums <= 0) {
		return -1;
	}

	com_thread_data_t *data = gethandle();
	if (data == NULL) {
		_com_error("get handle err");
		return -1;
	}

	for (int i=0; i<nums; ++i) {
		comspace::Appender *app = comspace::AppenderFactory::getAppender(dev[i]);
		if (app == NULL) {
			continue;
		}
		if (selflog) {
			u_int logid = comspace::createSelfLog(selflog);
			if (logid <= 0) {
				_com_error("create selflog %s err", selflog);
				return -1;
			}
			app->addMask(logid);
		}
		data->category->addAppender(app);
	}
	return 0;
}


com_logstat_t::com_logstat_t()
{
	sysevents = 16;
	userevents[0] = 0;
	ttydisp = 0;
}

int com_getlogid(char *selflog)
{
	if (selflog == NULL) return 0;
	return comspace::getLogId(selflog);
}

int com_reglogstat(const char *path, const char *file)
{
	return comspace::reg_logstat(com_getcategory()->logstat(), path, file);
}

int com_changelogstat()
{
	return comspace::reg_deal();
}

int com_changelogstat(com_logstat_t *logstat)
{
	if (logstat == NULL) return -1;
    comlog_set_log_level(logstat->sysevents);
	u_int syslvl = comspace::comlog_get_lower_syslevel(logstat->sysevents);
	return com_getcategory()->logstat()->setLevel(syslvl, logstat->userevents);
}

int set_comdevice_selflog(com_device_t *dev, const char *logname)
{
	char res[COM_SELFLEVELNUM][COM_MAXLEVELNAMESIZE];
	int items = comspace::xparserSelfLog(logname, res, sizeof(res)/sizeof(res[0]));
	//_com_debug("get selflog items[%d] aipos[%d]", items, i);
	for (int j=0; j<items; ++j) {
		//u_int id = comspace::getLogId(res[j]);
		u_int id = comspace::createSelfLog(res[j]);
		_com_debug("selflog[%d][%s]", id, res[j]);
		//if (id >= COM_USRLEVELSTART && id < COM_LEVELNUM) {
			COMLOG_ADDMASK(*dev, id);
			_com_debug("now self[%llx] add id[%d]", dev->log_mask, id);
		//}
	}
	return 0;
}


int com_registappender(const char * name, func_type getAppender, func_type tryAppender){
	return comspace :: AppenderFactory :: registAppender(name, getAppender, tryAppender);
}

int com_log_enabled(int level)
{
	if(comspace::com_get_log_switcher() != comspace :: __USING_COMLOG__) {
		return ul_log_enabled(level);
	}

	return !(com_getcategory()->logstat()->unAccept(level));
}

int com_log_enabled(const char *selflevel)
{
	int level = comspace::getLogId(selflevel);
	if (level == 0) {
		return 0;
	}

	return com_log_enabled(level);
}

int com_logstatus(){
	switch(comspace::com_get_log_switcher()){
		case comspace::__USING_COMLOG__:
			return USING_COMLOG;
		case comspace::__USING_ULLOG__:
			return USING_ULLOG;
		default:
			return LOG_NOT_DEFINED;
	}
	return LOG_NOT_DEFINED;
}

int comlog_get_log_level()
{
	return g_comlog_global_level;
}

void comlog_set_log_level(int log_level)
{
	g_comlog_global_level = log_level;
}

namespace comspace{
class Comlog_Globals {
public :
	Comlog_Globals() {
		comlog_get_def_layout();
		comlog_get_nameMg();
		SendSvr::comlog_get_gcheckSvr();
	}
	~Comlog_Globals() {
	}
};

static class Comlog_Globals comglobals;  /**<  com global init destroy */
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
