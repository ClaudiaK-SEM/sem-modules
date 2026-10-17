#include "DelayTrois.h"
//#include <mutex>

#include "math.h"
//#include "stdio.h"
#define ONEPI 3.14159265359f

using namespace gmpi;
//#define interpolationExtraSamples 3
//using namespace std;

static const double speed = 1.0;
static const float anti_denormal = 1e-18f;
inline void kill_denormal(float &p_sample ) { p_sample += anti_denormal;p_sample -= anti_denormal;};


REGISTER_PLUGIN ( DelayTrois, L"KX Delay v3" );

DelayTrois::~DelayTrois()
{
	// free the memory used by the delay buffer
	delete [] buffer;
}
DelayTrois::DelayTrois( IMpUnknown* host ) : MpBase( host )
,buffer(0)
{

	// Register pins.
	initializePin( 0, pinInput );
	initializePin( 1, pinFeedback );
	initializePin( 2, pinDelayms );
	initializePin( 3, pinMod );
	initializePin( 4, pinFXGain );
	initializePin( 5, pinPan );
	initializePin( 6, pinResetMemory );
	initializePin( 7, pinStOut );
	initializePin( 8, pinFXOutL );
	initializePin( 9, pinFXOutR );
	initializePin( 10, pinFXMono );
	initializePin( 11, on );
	initializePin( 12, sinc );
	//initializePin( 13, pinMix );



}
int32_t DelayTrois::open()
{

    //static std::mutex safeInit;
    //std::lock_guard<std::mutex> lock(safeInit);


	//int32_t need_initialise;
	//getHost()->allocateSharedMemory( L"CK77_Delay_SIN", (void**) &sin_tbl, -1, 10002*sizeof(float), need_initialise );

	//int32_t need_initialise1;
	//getHost()->allocateSharedMemory( L"CK77_Delay_COS", (void**) &cos_tbl, -1, 10002*sizeof(float), need_initialise1 );


    //if(need_initialise1)
    //{
			for (int i=0; i<10001; i++)
			{
			float cx=(float(i)*0.0001f);//1
            sin_tbl[i]=sinf(cx*ONEPI*0.5f)+0.0000001f;
            cos_tbl[i]=cosf(cx*ONEPI*0.5f)+0.0000001f;
            }

    //}



    if(buffer)
	{
        delete [] buffer;
		buffer = NULL;
	}

	buffer_size = (int) ( getSampleRate() * 10 );
    fbuffer_size = double(buffer_size);
	buffer = new float[buffer_size];
	memset(buffer, 0, buffer_size * sizeof(float) ); // clear buffer*/

    //CreateBuffer();
    count=0;

    m_id=0.0f;
	mul=getSampleRate()*0.01f;//480 pour 48k
    run_on=false;
    out_delay=0;

    //at_clock=-1;
    at_min=int(mul*0.1f);//48000*0.001=48;
    time_a=at_min;
    inc_at=1.0f/time_a;
    at=0.0f;


	return MpBase::open();
}
void DelayTrois::subProcess( int bufferOffset, int sampleFrames )
{
	// get pointers to in/output buffers.
	float* input	= bufferOffset + pinInput.getBuffer();
	float* feed	= bufferOffset + pinFeedback.getBuffer();
	float* d_time	= bufferOffset + pinDelayms.getBuffer();
	float* mod	= bufferOffset + pinMod.getBuffer();
	float* fx_gain	= bufferOffset + pinFXGain.getBuffer();
	float* pan	= bufferOffset + pinPan.getBuffer();
	float* st_out	= bufferOffset + pinStOut.getBuffer();
	float* fx_outL	= bufferOffset + pinFXOutL.getBuffer();
	float* fx_outR	= bufferOffset + pinFXOutR.getBuffer();
	float* fx_outM	= bufferOffset + pinFXMono.getBuffer();
	//float* mix	= bufferOffset + pinMix.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
	{


            if(*d_time !=m_d_time || *mod !=m_mod)
            {
            m_d_time=*d_time;
            mod2=m_mod=*mod;
            if(mod2>0.499f)mod2=0.499f;
            if(mod2<-0.499f)mod2=-0.499f;
            delay_size=double(*d_time*mul)+2.0;//S=0 to 1*1000ms (limit 5000ms)->100*480=48000+2
            d_count=double(count)-(delay_size+(mod2*2.0 *(delay_size-1.0)));//count-(delay time+mod)+(2spl min)
            }
            if(*feed !=m_feed)
            {
            limitfeed=m_feed=*feed;
            if(limitfeed>0.99f)limitfeed=0.99f;
            if(limitfeed<-0.99f)limitfeed=-0.99f;
            if(limitfeed<0.0f)limitfeed-=0.0001f;
            else
            limitfeed+=0.0001f;
            }




            //if(at<=inc_at)at=inc_at;

            float in=*input*at;
            //if(count>=buffer_size)count=0;
            //float res=*input+(limitfeed*out_delay);
            if(in<=-2.0f && in>=2.0f)in=(((float) rand() / (float) RAND_MAX) * 0.5f)*0.000001f;
            float res=in+(limitfeed*out_delay);
            kill_denormal(res);
            buffer[count]=res;
            count++;
            if(count>=buffer_size)count=0;

            if(d_count<0.0)d_count+=fbuffer_size;
            if(d_count>=fbuffer_size)d_count-=fbuffer_size;
            int id_count=int(d_count);
            double fd_count=d_count-double(id_count);

               if(sinc==1)
               {
               int id_count1=id_count+1;
               int id_count2=id_count+2;
               if(id_count1>=buffer_size)id_count1-=buffer_size;
               if(id_count2>=buffer_size)id_count2-=buffer_size;
               out_delay=*fx_gain*hermite(m_id,buffer[id_count],buffer[id_count1],buffer[id_count2],fd_count);
               }
               else
               {
               int id_count1=id_count+1;
               if(id_count1>=buffer_size)id_count1-=buffer_size;
               out_delay=*fx_gain*float(buffer[id_count]+fd_count*(buffer[id_count1]-buffer[id_count]));
               }

            m_id=buffer[id_count];
            d_count+=speed;


            if(*pan !=m_pan)
            {
            float x=m_pan=*pan;
            if(x>=1.0f)x=1.0f;
            if(x<0.0f)x=0.0f;
            float float_x=x*10000.0f;
            int int_x=int(float_x);
            float fract_x=float_x-float(int_x);
            l_pan =cos_tbl[int_x]+(fract_x*(cos_tbl[int_x+1]-cos_tbl[int_x]));
            r_pan =sin_tbl[int_x]+(fract_x*(sin_tbl[int_x+1]-sin_tbl[int_x]));
            }

            //if(out_delay<=-2.0f && out_delay>=2.0f)out_delay=(((float) rand() / (float) RAND_MAX) * 0.5f)*0.000001f;
            *fx_outL=*st_out*out_delay*l_pan;
            *fx_outR=*st_out*out_delay*r_pan;
            *fx_outM=out_delay;

            if(at<1.0f && *fx_gain>0)at+=inc_at;
            if(at>=1.0f)at=1.0f;



		++input;
		++feed;
		++d_time;
		++mod;
		++fx_gain;
		++pan;
		++st_out;
		++fx_outL;
		++fx_outR;
		++fx_outM;
		//++mix;
	}
}
void DelayTrois::subProcess_0( int bufferOffset, int sampleFrames )
{

	float* fx_outL	= bufferOffset + pinFXOutL.getBuffer();
	float* fx_outR	= bufferOffset + pinFXOutR.getBuffer();
	float* fx_outM	= bufferOffset + pinFXMono.getBuffer();

	for( int s = sampleFrames; s > 0; --s )
    {


        *fx_outL=0.0f;
        *fx_outR=0.0f;
        *fx_outM=0.0f;

		++fx_outL;
		++fx_outR;
		++fx_outM;
    }

}
void DelayTrois::onSetPins(void)
{

    //if(pinResetMemory.isStreaming() && pinResetMemory !=0.f)run_on=false;
    //if(on.isUpdated() && on==1)run_on=false;

    if(on.isUpdated() && on==1)
    {
    run_on=false;
    //ClearBuffer();
    //memset(buffer, 0, buffer_size * sizeof(float) );
    }




    if(pinResetMemory.isStreaming() && pinResetMemory !=0.f)
    {
    run_on=false;
    //ClearBuffer();
    //memset(buffer, 0, buffer_size * sizeof(float) );
    }




    //if(on==1 && pinFXGain > 0.0f && pinFXGain.isStreaming())
    if(on==1 &&  pinFXGain > 0.0f)
    {



        if( run_on==false)
        {
            run_on=true;
            //at_clock=-1;
            at=0.0f;
            m_id=0.0f;
            count=0;
            out_delay=0;

            m_feed=pinFeedback-0.0000001f;
            m_d_time=pinDelayms-0.0000001f;
            m_mod=pinMod-0.0000001f;
            m_pan=pinPan-0.0000001f;

            //ClearBuffer();
            memset(buffer, 0, buffer_size * sizeof(float) );

            pinFXOutL.setStreaming(true);
            pinFXOutR.setStreaming(true);
            pinFXMono.setStreaming(true);
            SET_PROCESS(&DelayTrois::subProcess);
            setSleep(false);
        }




    }
    else
    {
    run_on=false;
    //at_clock=-1;
    //at=0.0f;
    //m_id=0.0f;
    //out_delay=0;
    //memset(buffer, 0, buffer_size * sizeof(float) ); // clear buffer
    //ClearBuffer();
    pinFXOutL.setStreaming(false);
    pinFXOutR.setStreaming(false);
    pinFXMono.setStreaming(false);
    SET_PROCESS(&DelayTrois::subProcess_0);
    setSleep(true);
    }


}

float DelayTrois::hermite(float y0, float y1, float y2, float y3, double frac)
{
// 4-point, 3rd-order Hermite (x-form)
double c0 = y1;
double c1 = 1/2.0*(y2-y0);
double c2 = y0 - 5/2.0*y1 + 2*y2 - 1/2.0*y3;
double c3 = 1/2.0*(y3-y0) + 3/2.0*(y1-y2);
return float(((c3*frac+c2)*frac+c1)*frac+c0)*0.95f;
}

/*void DelayTrois::CreateBuffer()
{

 	//buffer_size = (int) ( getSampleRate() * 10 );

	//buffer = new float[buffer_size];
	//memset(buffer, 0, buffer_size * sizeof(float) ); // clear buffer

    buffer_size = (int)(getSampleRate() * 10);
    fbuffer_size = double(buffer_size);

    //if (buffer_size < 0)
        //buffer_size = 0;

    //if (buffer_size > getSampleRate() * 10)	// limit to 10 s sample
        //buffer_size = (int)getSampleRate() * 10;

    // Need at least 3 samples at begining for wrapping interpolation samples.
    //padded_buffer_size = interpolationExtraSamples + buffer_size;

    // Add some samples off-end to simplify interpolation.
   //int allocatedSize = padded_buffer_size + interpolationExtraSamples;

    buffer.assign(buffer_size, 0.0f);

    // ensure we aren't accessing data outside buffer
    //count = 0;
}

void DelayTrois::ClearBuffer()
{
    std::fill(buffer.begin(), buffer.end(), 0.0f);
}*/



