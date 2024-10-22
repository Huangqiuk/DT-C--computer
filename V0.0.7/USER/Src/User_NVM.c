#include "USER_Common.h"
#include "fds.h"

//�����ļ�ID�͸��ļ������ļ�¼��KEY
#define NVM_FILE                       (0x1000)//�ļ�ID
#define NVM_KEY                (0x1001)//��¼KEY���ü�¼��ŵ��ļ�ID=0X1000

//��¼m_version_record�����ݣ�Ҳ���Զ���Ϊ���飬����ע��һ��Ҫ4�ֽڶ���
#define DATA_LEN 20
typedef struct
{
	  char  data[DATA_LEN];
	
}__attribute__((aligned(4)))NVM_Data_t;


//��ʼ��Ĭ��ֵ
static NVM_Data_t NVM_DATA =
{
	.data = {0x00}
};


//��¼
static fds_record_t const m_version_record =
{
    .file_id           = NVM_FILE,
    .key               = NVM_KEY,
    .data.p_data       = &NVM_DATA,
    //��¼�ĳ��ȱ�����4�ֽڣ��֣�Ϊ��λ
    .data.length_words = (sizeof(NVM_DATA) + 3) / sizeof(uint32_t),
};


//FDS�첽������־�ṹ��
typedef struct
{
    bool version_record_update;    //version_record��¼���±�־
	  bool desp_record_update;       //desp_record��¼���±�־
	  bool read;       //����¼��־
	  bool gc;         //��Ƭ�ռ���־
    bool busy;       //FDSæ��־
}my_fds_info_t;
//����FDS�첽������־�ṹ��
static my_fds_info_t my_fds_info;

//FDS�¼�������
static void fds_evt_handler(fds_evt_t const * p_evt)
{
    //�ж��¼�����
    switch (p_evt->id)
    {
        case FDS_EVT_INIT://FDS��ʼ���¼�
					  //��ʼ���ɹ�
            if (p_evt->result == NRF_SUCCESS)
            {
                my_fds_info.busy = false;
            }
            break;
        //FDSд��¼�¼�
        case FDS_EVT_WRITE:
        {
            //д��¼�ɹ�
					  if (p_evt->result == NRF_SUCCESS)
            {
                my_fds_info.busy = false;
            }
        } break;
				//FDS���¼�¼�¼�
        case FDS_EVT_UPDATE:
        {
            //д��¼�ɹ�
					  if (p_evt->result == NRF_SUCCESS)
            {
                my_fds_info.busy = false;
            }
        } break;
				//FDS��Ƭ�����¼�
        case FDS_EVT_GC:
        {
            //��Ƭ����ɹ�
					  if (p_evt->result == NRF_SUCCESS)
            {
							  my_fds_info.busy = false;
            }
        } break;
        default:
            break;
    }
}

//�ȴ�FDS��ʼ�����
static void wait_for_fds_ready(void)
{
    while (my_fds_info.busy)
    {
        (void) sd_app_evt_wait();
    }
}
//������
//�������������ݺͳ���
void read_NVM_record(uint8_t *data, uint8_t* len)
{
		ret_code_t rc;
	  //���岢��ʼ����¼�������ṹ�����
	  fds_record_desc_t desc = {0};
	  //���岢��ʼ����¼�������ƽṹ�����
    fds_find_token_t  tok  = {0};
		
	  //����tok����ͷ����
		memset(&tok, 0x00, sizeof(fds_find_token_t));
		//��DEVICE_FILE�ļ��в��Ҽ�¼m_version_record
		rc = fds_record_find(NVM_FILE, NVM_KEY, &desc, &tok);
		//���ҵ���¼�󣬶�ȡ��¼����
		if (rc == NRF_SUCCESS)
		{
				fds_flash_record_t temp = {0};
				//�򿪼�¼��ȡ��¼����
				rc = fds_record_open(&desc, &temp);
				APP_ERROR_CHECK(rc);
				static NVM_Data_t temp_desp;
				//������¼����
				memcpy(&temp_desp, temp.p_data, sizeof(NVM_Data_t));
				//���ڴ�ӡ��¼����
				memcpy(data, &temp_desp.data, DATA_LEN);
				*len = DATA_LEN;
				
				for(uint8_t i=0; i<*len; i++){
					NRF_LOG_INFO("%c - %x", data[i], data[i]);
				}
				NRF_LOG_HEXDUMP_DEBUG(data, *len);
				
				//��ȡ�󣬹رռ�¼
				rc = fds_record_close(&desc);
				APP_ERROR_CHECK(rc);
		}else{
				NRF_LOG_INFO("FDS No Save");
		}
}
//��������
void updata_NVM_Record(uint8_t *data, uint8_t len)
{

	ret_code_t rc;
	//���岢��ʼ����¼�������ṹ�����
	fds_record_desc_t desc = {0};
	//���岢��ʼ����¼�������ƽṹ�����
  fds_find_token_t  tok  = {0};	
	
	if(len > DATA_LEN)
	{
		data = NULL;
		return;
	}
	
	//����tok����ͷ����
	memset(&tok, 0x00, sizeof(fds_find_token_t));
	my_fds_info.busy = true;
	//��DEVICE_FILE�ļ��в��Ҽ�¼m_version_record
	rc = fds_record_find(NVM_FILE, NVM_KEY, &desc, &tok);	
	if (rc == NRF_SUCCESS)
	{
     my_fds_info.version_record_update = false;
		 //���¼�¼m_version_record
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

//----------------------��ʼ��------------------
void User_NVM_Init()
{
	ret_code_t rc;
	//���岢��ʼ����¼�������ṹ�����
	fds_record_desc_t desc = {0};
	//���岢��ʼ����¼�������ƽṹ�����
  fds_find_token_t  tok  = {0};
	
	//ע��FDS�¼��ص���������FS�¼�
	(void)fds_register(fds_evt_handler);
	
	my_fds_info.busy = true;
	rc = fds_init();//��ʼ��FDS
  APP_ERROR_CHECK(rc);//�ô�����ģ���麯������ֵ	
	//FDS��ʼ�����첽�ģ����Ҫ�ȴ�FDS��ʼ�����
  wait_for_fds_ready();	

	//����tok����ͷ����
	memset(&tok, 0x00, sizeof(fds_find_token_t));
	//��DEVICE_FILE�ļ��в��Ҽ�¼m_version_record
	rc = fds_record_find(NVM_FILE, NVM_KEY, &desc, &tok);								
	//û�в��ҵ�m_version_record��¼��д���¼
	if (rc != NRF_SUCCESS)
	{
		my_fds_info.busy = true;
	  rc = fds_record_write(&desc, &m_version_record);
	  APP_ERROR_CHECK(rc);
	  wait_for_fds_ready();
	}else{
		
	}
}
