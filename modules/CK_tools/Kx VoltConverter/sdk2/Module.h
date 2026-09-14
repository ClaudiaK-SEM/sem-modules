#if !defined(_SEModule_h_inc_)
#define _SEModule_h_inc_

#include "SEModule_base.h"



class Module : public SEModule_base
{
public:
	Module(seaudioMasterCallback2 seaudioMaster, void *p_resvd1);
	~Module();

//	virtual bool getModuleProperties (SEModuleProperties* properties);
	static void getModuleProperties(SEModuleProperties* properties);
	virtual bool getPinProperties (long index, SEPinProperties* properties);
	void SE_CALLING_CONVENTION sub_process(long buffer_offset, long sampleframes );
//	void SE_CALLING_CONVENTION sub_process_cruise(long buffer_offset, long sampleframes );
	void OnPlugStateChange(SEPin *pin);
//	void OnGuiNotify( int p_user_msg_id, int p_size, void *p_data );
	virtual void open();

private:
	//void Freq();
    float in;
    float out;
    //float volt;
    //float m_volt;
};
#endif
