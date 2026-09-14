// prevent MS CPP - 'swprintf' was declared deprecated warning
#if defined(_MSC_VER)
  #define _CRT_SECURE_NO_DEPRECATE
  #pragma warning(disable : 4996)
#endif

#include "KxConvGui.h"
#include "math.h"
//#include <cmath>

REGISTER_GUI_PLUGIN( KxConvGui, L"KxConvGui" );

KxConvGui::KxConvGui( IMpUnknown* host ) : MpGuiBase(host)
{

	initializePin( 0, pinA, static_cast<MpGuiBaseMemberPtr>( &KxConvGui::onAChanged ) );
	initializePin( 1, pinB, static_cast<MpGuiBaseMemberPtr>( &KxConvGui::onBChanged ) );


}
void KxConvGui::onAChanged()
{
 //pinB=pinA;
}
void KxConvGui::onBChanged()
{
//pinA=pinB;
pinA=(powf(2.0f,(pinB*10.0f)-9.999f)-0.00095f); //exp de 1.0f for animation
 //exp de 1.0f for animation
}
