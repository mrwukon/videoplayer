

#ifndef PCMVOLUMECONTROL_H
#define PCMVOLUMECONTROL_H

class PcmVolumeControl
{
public:
    PcmVolumeControl();


    static void RaiseVolume(char* buf, int size, int uRepeat, double vol);

};




#endif // PCMVOLUMECONTROL_H
