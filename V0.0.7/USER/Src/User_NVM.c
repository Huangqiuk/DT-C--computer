#include "USER_Common.h"
#include "fds.h"

//定义文件ID和该文件包含的记录的KEY
#define NVM_FILE                       (0x1000)//文件ID
#define NVM_KEY                (0x1001)//记录KEY，该记录存放的文件ID=0X1000

//记录m_version_record的内容，也可以定义为数组，但是注意一定要4字节对齐
#define DATA_LEN 20
typedef struct
{
	  char  data[DATA_LEN];
	
}__attribute__((aligned(4)))NVM_Data_t;


//初始化默认值
static NVM_Data_t NVM_DATA =
{
	.data = {0x00}
};


//记录
static fds_record_t const m_version_record =
{
    .file_id           = NVM_FILE,
    .key               = NVM_KEY,
    .data.p_data       = &NVM_DATA,
    //记录的长度必须以4字节（字）为单位
    .data.length_words = (sizeof(NVM_DATA) + 3) / sizeof(uint32_t),
};


//FDS异步操作标志结构体
typedef struct
{
    bool version_record_update;    //version_record记录更新标志
	  bool desp_record_update;       //desp_record记录更新标志
	  bool read;       //读记录标志
	  bool gc;         //碎片收集标志
    bool busy;       //FDS忙标志
}my_fds_info_t;
//定义FDS异步操作标志结构体
static my_fds_info_t my_fds_info;

//FDS事件处理函数
static void fds_evt_handler(fds_evt_t const * p_evt)
{
    //判断事件类型
    switch (p_evt->id)
    {
        case FDS_EVT_INIT://FDS初始化事件
					  //初始化成功
            if (p_evt->result == NRF_SUCCESS)
            {
                my_fds_info.busy = false;
            }
            break;
        //FDS写记录事件
        case FDS_EVT_WRITE:
        {
            //写记录成功
					  if (p_evt->result == NRF_SUCCESS)
            {
                my_fds_info.busy = false;
            }
        } break;
				//FDS更新记录事件
        case FDS_EVT_UPDATE:
        {
            //写记录成功
					  if (p_evt->result == NRF_SUCCESS)
            {
                my_fds_info.busy = false;
            }
        } break;
				//FDS碎片整理事件
        case FDS_EVT_GC:
        {
            //碎片整理成功
					  if (p_evt->result == NRF_SUCCESS)
            {
							  my_fds_info.busy = false;
            }
        } break;
        default:
            break;
    }
}

//等待FDS初始化完成
static void wait_for_fds_ready(void)
{
    while (my_fds_info.busy)
    {
        (void) sd_app_evt_wait();
    }
}
//读数据
//读到的数据内容和长度
void read_NVM_record(uint8_t *data, uint8_t* len)
{
		ret_code_t rc;
	  //定义并初始化记录描述符结构体变量
	  fds_record_desc_t desc = {0};
	  //定义并初始化记录查找令牌结构体变量
    fds_find_token_t  tok  = {0};
		
	  //清零tok，从头查找
		memset(&tok, 0x00, sizeof(fds_find_token_t));
		//在DEVICE_FILE文件中查找记录m_version_record
		rc = fds_record_find(NVM_FILE, NVM_KEY, &desc, &tok);
		//查找到记录后，读取记录内容
		if (rc == NRF_SUCCESS)
		{
				fds_flash_record_t temp = {0};
				//打开记录读取记录内容
				rc = fds_record_open(&desc, &temp);
				APP_ERROR_CHECK(rc);
				static NVM_Data_t temp_desp;
				//拷贝记录内容
				memcpy(&temp_desp, temp.p_data, sizeof(NVM_Data_t));
				//串口打印记录内容
				memcpy(data, &temp_desp.data, DATA_LEN);
				*len = DATA_LEN;
				
				for(uint8_t i=0; i<*len; i++){
					NRF_LOG_INFO("%c - %x", data[i], data[i]);
				}
				NRF_LOG_HEXDUMP_DEBUG(data, *len);
				
				//读取后，关闭记录
				rc = fds_record_close(&desc);
				APP_ERROR_CHECK(rc);
		}else{
				NRF_LOG_INFO("FDS No Save");
		}
}
//更新数据
void updata_NVM_Record(uint8_t *data, uint8_t len)
{

	ret_code_t rc;
	//定义并初始化记录描述符结构体变量
	fds_record_desc_t desc = {0};
	//定义并初始化记录查找令牌结构体变量
  fds_find_token_t  tok  = {0};	
	
	if(len > DATA_LEN)
	{
		data = NULL;
		return;
	}
	
	//清零tok，从头查找
	memset(&tok, 0x00, sizeof(fds_find_token_t));
	my_fds_info.busy = true;
	//在DEVICE_FILE文件中查找记录m_version_record
	rc = fds_record_find(NVM_FILE, NVM_KEY, &desc, &tok);	
	if (rc == NRF_SUCCESS)
	{
     my_fds_info.version_record_update = false;
		 //更新记录m_version_record
		 memset(NVM_DATA.data, 0, DATA_LEN);
		 memcpy(NVM_DATA.data, data, len);
		 
		 rc = fds_record_update(&desc, &m_version_record);
		 APP_ERROR_CHECK(rc);
		 wait_for_fds_ready();
	}
	else
	{
	
	}
}

//----------------------初始化------------------
void User_NVM_Init()
{
	ret_code_t rc;
	//定义并初始化记录描述符结构体变量
	fds_record_desc_t desc = {0};
	//定义并初始化记录查找令牌结构体变量
  fds_find_token_t  tok  = {0};
	
	//注册FDS事件回调函数接收FS事件
	(void)fds_register(fds_evt_handler);
	
	my_fds_info.busy = true;
	rc = fds_init();//初始化FDS
  APP_ERROR_CHECK(rc);//用错误处理模块检查函数返回值	
	//FDS初始化是异步的，因此要等待FDS初始化完成
  wait_for_fds_ready();	

	//清零tok，从头查找
	memset(&tok, 0x00, sizeof(fds_find_token_t));
	//在DEVICE_FILE文件中查找记录m_version_record
	rc = fds_record_find(NVM_FILE, NVM_KEY, &desc, &tok);								
	//没有查找到m_version_record记录，写入记录
	if (rc != NRF_SUCCESS)
	{
		my_fds_info.busy = true;
	  rc = fds_record_write(&desc, &m_version_record);
	  APP_ERROR_CHECK(rc);
	  wait_for_fds_ready();
	}else{
		
	}
}
