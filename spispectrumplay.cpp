/*
 * Copyright (c) 2012-2026 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 1901 rue Gilford, #53
 * Montreal, QC, H2H 1G8
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
 /*
	BASS spectrum analyser example
	Copyright (c) 2002-2012 Un4seen Developments Ltd.
*/

#include <windows.h>
#include <windowsx.h> //dj-oifii
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include "bassasio.h"
#include "bass.h"

#include "resource.h"

#include <string>
using namespace std;
#include <assert.h>
#include <map>
map<string,int> global_asiodevicemap;
//2020oct08, spi, begin
bool global_isasio = false;
map<string,int> global_nonasiodevicemap;
FILE* pFILE = NULL;
//2020oct08, spi, end

//2022jan08, spi, begin
//global_audiodevicename_default is initialize here BUT will be set later when detecting installed/default devices
//asio device should be fully and well specified, global_audiodevicename_default is not used with asio device matching
string global_audiodevicename_default = "Speakers (USB AUDIO  CODEC)"; //"E-MU ASIO"; //"Speakers (2- E-MU E-DSP Audio Processor (WDM))"
//2022jan08, spi, end

//2022may14, spi, begin
bool global_bloopmode = false;
//2022may14, spi, end

//#define SPECWIDTH 368	// display width
#define SPECWIDTH 400	// display width
#define SPECHEIGHT 127	// height (changing requires palette adjustments too)

BYTE global_alpha=200;

string global_filename;
float global_fSecondsPlay; //negative for playing only once
string global_audiodevicename; 
int global_outputAudioChannelSelectors[2]; //int outputChannelSelectors[1];
BASS_CHANNELINFO global_BASS_CHANNELINFO;
int global_deviceid=0;
DWORD global_timer=0;
int global_x=200;
int global_y=200;

HWND win=NULL;
DWORD timer=0;

DWORD chan;

HDC specdc=0;
HBITMAP specbmp=0;
BYTE *specbuf;

//int specmode=0; //spectrum mode
int specmode=0; //spectrum mode
int specpos=0; // marker pos for 2nd mode

//spi, fix, begin
BYTE* asiobuf=NULL; // buffer for the sample data
DWORD asiobuflen; // buffer length
HSTREAM bufstream;
#define FFTSIZE	2048
//spi, fix, end

//2021sept22, spi, begin
#define IDT_TIMER_TITLEBAR	1
string global_windowtitle = "spispectrumplay - click sets vol, shift-click changes mode or pos)";
string global_filename_tobedisplayed = ""; //set after opening audio filename in WM_CREATE
int global_windowtitle_notificationtime_ms = 2000;
QWORD global_length_inbyte = 0;
double global_length_insec = 0.0;
float global_fvolume = 1.0;
int xmousepos = 0;
int ymousepos = 0;

std::string tail(std::string const& source, size_t const length) {
  if (length >= source.size()) { return source; }
  return source.substr(source.size() - length);
} // tail

class MouseTrackEvents
{
    bool m_bMouseTracking;

public:
    MouseTrackEvents() : m_bMouseTracking(false)
    {
    }
    
    void OnMouseMove(HWND hwnd)
    {
        if (!m_bMouseTracking)
        {
            // Enable mouse tracking.
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = hwnd;
            tme.dwFlags = TME_HOVER | TME_LEAVE;
            tme.dwHoverTime = HOVER_DEFAULT;
            TrackMouseEvent(&tme);
            m_bMouseTracking = true;
        }
    }
    void Reset(HWND hwnd)
    {
        m_bMouseTracking = false;
    }
};
//2021sept22, spi, end

void CALLBACK StopPlayingFile(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	PostMessage(win, WM_DESTROY, 0, 0);
}

// display error messages
void Error(const char* text)
{
	char mes[200];
	//sprintf(mes,"%s\n(error code: %d)",text,BASS_ErrorGetCode());
	sprintf_s(mes,200,"Error(%d/%d): %s\n",BASS_ErrorGetCode(),BASS_ASIO_ErrorGetCode(),text);
	MessageBox(win,mes,0,0);
}

// ASIO function
DWORD CALLBACK AsioProc(BOOL input, DWORD channel, void *buffer, DWORD length, void *user)
{
	DWORD c=BASS_ChannelGetData((DWORD)user,buffer,length);
	if (c==-1) c=0; // an error, no data
	//spi, fix, begin
	if (c<asiobuflen) 
	{
		memmove(asiobuf, asiobuf+c, asiobuflen-c); // shift the old data in the buffer to make space for the new
		memcpy(asiobuf+asiobuflen-c, buffer, c); // add the new data
	} 
	else
	{
		memcpy(asiobuf, buffer, asiobuflen); // copy the data to the buffer
	}
	//spi, fix, end
	return c;
}

DWORD CALLBACK BufStreamProc(HSTREAM handle, void *buffer, DWORD length, void *user)
{
	if (length>asiobuflen) length=asiobuflen; // just in case
	memcpy(buffer, asiobuf, length); // copy the data from the buffer
	return length;
}

BOOL PlayFile(const char* filename)
{
	//2020oct08, spi, begin
	if(global_isasio==false)
	{
		//2022july02, spi, begin
		/* //original code
		//non-asio device
		if (!(chan=BASS_StreamCreateFile(FALSE,filename,0,0,BASS_SAMPLE_LOOP))
			&& !(chan=BASS_MusicLoad(FALSE,filename,0,0,BASS_MUSIC_RAMP|BASS_SAMPLE_LOOP,1))) 
		*/
		//2022july02, modified code
		//spinote, BASS_MUSIC_PRESCAN
		//spinote, BASS_SAMPLE_FLOAT
		//spinote, BASS_MUSIC_RAMPS
		//spinote, BASS_MusicLoad() functon's last entry parameter is a flag, 0 or 1, related to the freq Sample rate to render/play the MOD music at... 0 = the rate specified in the BASS_Init call, 1 = the device's current output rate (or the BASS_Init rate if that is not available).
		//spinote, 2022july02 briefly test for non-asio to see if stable considering new flags introduced that are now very similar to the non-asio code branch
		//non-asio device
		//2022july03, spi, begin
		//if (!(chan = BASS_StreamCreateFile(FALSE, filename, 0, 0, BASS_SAMPLE_LOOP | BASS_SAMPLE_FLOAT))
		if (!(chan = BASS_StreamCreateFile(FALSE, filename, 0, 0, BASS_SAMPLE_LOOP | BASS_SAMPLE_FLOAT | BASS_STREAM_PRESCAN))
			//2022july03, spi, end
			&& !(chan = BASS_MusicLoad(FALSE, filename, 0, 0, BASS_SAMPLE_LOOP | BASS_SAMPLE_FLOAT| BASS_MUSIC_RAMPS | BASS_MUSIC_PRESCAN, 0))) //1)))
		//2022july02, spi, end
		{
			Error("Can't play file");
			return FALSE; // Can't load the file
		}
	}
	else
	{
		//asio device
		if (!(chan=BASS_StreamCreateFile(FALSE,filename,0,0,BASS_SAMPLE_LOOP|BASS_STREAM_DECODE|BASS_SAMPLE_FLOAT))
			&& !(chan=BASS_MusicLoad(FALSE,filename,0,0,BASS_SAMPLE_LOOP|BASS_STREAM_DECODE|BASS_SAMPLE_FLOAT|BASS_MUSIC_RAMPS|BASS_MUSIC_PRESCAN,0))) 
		{
			Error("Can't play file");
			return FALSE; // Can't load the file
		}
	}

	if(global_fSecondsPlay<=0)
	{
		QWORD length_byte=BASS_ChannelGetLength(chan,BASS_POS_BYTE);
		global_fSecondsPlay=BASS_ChannelBytes2Seconds(chan,length_byte);
	}
	global_timer=timeSetEvent(global_fSecondsPlay*1000,25,(LPTIMECALLBACK)&StopPlayingFile,0,TIME_ONESHOT);

	if(global_isasio==false)
	{
		//non-asio device
		BASS_ChannelPlay(chan,FALSE);
	}
	else
	{
		//asio device
		// setup ASIO stuff
		if (!BASS_ASIO_Init(global_deviceid,BASS_ASIO_THREAD))
			Error("Can't initialize ASIO device");
		if(!BASS_ChannelGetInfo(chan,&global_BASS_CHANNELINFO))
		{
			Error("BASS_ChannelGetInfo fails");
		}
		//spi, fix, begin
		bufstream=BASS_StreamCreate(global_BASS_CHANNELINFO.freq, global_BASS_CHANNELINFO.chans, BASS_SAMPLE_FLOAT|BASS_STREAM_DECODE, BufStreamProc, 0); // create a custom stream with the same format
		asiobuflen=FFTSIZE*global_BASS_CHANNELINFO.chans*sizeof(float); // FFTSIZE = xxx part of the BASS_DATA_FFTxxx flag used
		asiobuf=(BYTE*)malloc(asiobuflen); // allocate the buffer
		memset(asiobuf, 0, asiobuflen);
		//spi, fix, end
		//BASS_ASIO_ChannelEnable(0,0,&AsioProc,(void*)chan); // enable 1st output channel...
		if(!BASS_ASIO_ChannelEnable(0,global_outputAudioChannelSelectors[0],&AsioProc,(void*)chan)) // enable 1st output channel...
		{
			Error("BASS_ASIO_ChannelEnable fails");
		}
		//for (a=1;a<i.chans;a++)
		//	BASS_ASIO_ChannelJoin(0,a,0); // and join the next channels to it
		if(!BASS_ASIO_ChannelJoin(0,global_outputAudioChannelSelectors[1],global_outputAudioChannelSelectors[0]))
		{
			Error("BASS_ASIO_ChannelJoin fails");
		}
		if (global_BASS_CHANNELINFO.chans==1) BASS_ASIO_ChannelEnableMirror(1,0,0); // mirror mono channel to form stereo output
		//BASS_ASIO_ChannelSetFormat(0,0,BASS_ASIO_FORMAT_FLOAT); // set the source format (float)
		if(!BASS_ASIO_ChannelSetFormat(0,global_outputAudioChannelSelectors[0],BASS_ASIO_FORMAT_FLOAT)) // set the source format (float)
		{
			Error("BASS_ASIO_ChannelSetFormat fails");
		}
		//BASS_ASIO_ChannelSetRate(0,0,i.freq); // set the source rate
		if(!BASS_ASIO_ChannelSetRate(0,global_outputAudioChannelSelectors[0],global_BASS_CHANNELINFO.freq)) // set the source rate
		{
			Error("BASS_ASIO_ChannelSetRate fails");
		}
		if(!BASS_ASIO_SetRate(global_BASS_CHANNELINFO.freq)) // try to set the device rate too (saves resampling)
		{
			Error("BASS_ASIO_SetRate fails");
		}
		if (!BASS_ASIO_Start(0)) // start output using default buffer/latency
			Error("Can't start ASIO output");
	}
	return TRUE;
}

// select a file to play, and play it
BOOL PlayFile()
{
	char file[MAX_PATH]="";
	OPENFILENAME ofn={0};
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=win;
	ofn.nMaxFile=MAX_PATH;
	ofn.lpstrFile=file;
	ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_EXPLORER;
	ofn.lpstrTitle="Select a file to play";
	ofn.lpstrFilter="playable files\0*.mo3;*.xm;*.mod;*.s3m;*.it;*.mtm;*.umx;*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*.aif\0All files\0*.*\0\0";
	if (!GetOpenFileName(&ofn)) return FALSE;
	
	return PlayFile(file);
}

// update the spectrum display - the interesting bit :)
void CALLBACK UpdateSpectrum(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	HDC dc;
	int x,y,y1;

	if (specmode==3) 
	{ // waveform
		int c;
		float *buf;
		BASS_CHANNELINFO ci;
		memset(specbuf,0,SPECWIDTH*SPECHEIGHT);
		BASS_ChannelGetInfo(chan,&ci); // get number of channels
		//buf=alloca(ci.chans*SPECWIDTH*sizeof(float)); // allocate buffer for data
		buf=(float*)alloca(ci.chans*SPECWIDTH*sizeof(float)); // allocate buffer for data
		//spi, fix, begin
		if (global_isasio == false)
		{
			BASS_ChannelGetData(chan,buf,(ci.chans*SPECWIDTH*sizeof(float))|BASS_DATA_FLOAT); // get the sample data (floating-point to avoid 8 & 16 bit processing)
		}
		else
		{
			BASS_ChannelGetData(bufstream, buf, (ci.chans * SPECWIDTH * sizeof(float)) | BASS_DATA_FLOAT); // get the sample data (floating-point to avoid 8 & 16 bit processing)
		}
		//spi, fix, end
		for (c=0;c<ci.chans;c++) 
		{
			for (x=0;x<SPECWIDTH;x++) 
			{
				int v=(1-buf[x*ci.chans+c])*SPECHEIGHT/2; // invert and scale to fit display
				if (v<0) v=0;
				else if (v>=SPECHEIGHT) v=SPECHEIGHT-1;
				if (!x) y=v;
				do { // draw line from previous sample...
					if (y<v) y++;
					else if (y>v) y--;
					specbuf[y*SPECWIDTH+x]=c&1?127:1; // left=green, right=red (could add more colours to palette for more chans)
				} while (y!=v);
			}
		}
	} 
	else 
	{
		float fft[1024];
		//spi, fix, begin
		if (global_isasio == false)
		{
			BASS_ChannelGetData(chan,fft,BASS_DATA_FFT2048); // get the FFT data
		}
		else
		{
			BASS_ChannelGetData(bufstream, fft, BASS_DATA_FFT2048); // get the FFT data
		}
		//spi, fix, end
		if (!specmode) { // "normal" FFT
			memset(specbuf,0,SPECWIDTH*SPECHEIGHT);
			for (x=0;x<SPECWIDTH/2;x++) 
			{
#if 1
				y=sqrt(fft[x+1])*3*SPECHEIGHT-4; // scale it (sqrt to make low values more visible)
#else
				y=fft[x+1]*10*SPECHEIGHT; // scale it (linearly)
#endif
				if (y>SPECHEIGHT) y=SPECHEIGHT; // cap it
				if (x && (y1=(y+y1)/2)) // interpolate from previous to make the display smoother
					while (--y1>=0) specbuf[y1*SPECWIDTH+x*2-1]=y1+1;
				y1=y;
				while (--y>=0) specbuf[y*SPECWIDTH+x*2]=y+1; // draw level
			}
		} 
		else if (specmode==1) 
		{ // logarithmic, acumulate & average bins
			int b0=0;
			memset(specbuf,0,SPECWIDTH*SPECHEIGHT);
#define BANDS 28
			for (x=0;x<BANDS;x++) 
			{
				float peak=0;
				int b1=pow(2,x*10.0/(BANDS-1));
				if (b1>1023) b1=1023;
				if (b1<=b0) b1=b0+1; // make sure it uses at least 1 FFT bin
				for (;b0<b1;b0++)
					if (peak<fft[1+b0]) peak=fft[1+b0];
				y=sqrt(peak)*3*SPECHEIGHT-4; // scale it (sqrt to make low values more visible)
				if (y>SPECHEIGHT) y=SPECHEIGHT; // cap it
				while (--y>=0)
					memset(specbuf+y*SPECWIDTH+x*(SPECWIDTH/BANDS),y+1,SPECWIDTH/BANDS-2); // draw bar
			}
		} else 
		{ // "3D"
			for (x=0;x<SPECHEIGHT;x++) 
			{
				y=sqrt(fft[x+1])*3*127; // scale it (sqrt to make low values more visible)
				if (y>127) y=127; // cap it
				specbuf[x*SPECWIDTH+specpos]=128+y; // plot it
			}
			// move marker onto next position
			specpos=(specpos+1)%SPECWIDTH;
			for (x=0;x<SPECHEIGHT;x++) specbuf[x*SPECWIDTH+specpos]=255;
		}
	}

	// update the display
	dc=GetDC(win);
	BitBlt(dc,0,0,SPECWIDTH,SPECHEIGHT,specdc,0,0,SRCCOPY);
	ReleaseDC(win,dc);
}

// window procedure
long FAR PASCAL SpectrumWindowProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
	//2021sept22, spi, begin
	MouseTrackEvents mouseTrack;
	const int maxchar = 8192;
	char charbuf[maxchar];
	string titlebartext ="";
	int nmaxchartobedisplayed = 5; //will be properly set in WM_CREATE
	std::size_t found=0;
	float fvolume = global_fvolume;
	string trackposition = "";
	//2022sept22, spi, end

	switch (m) {
		case WM_PAINT:
			if (GetUpdateRect(h,0,0)) {
				PAINTSTRUCT p;
				HDC dc;
				if (!(dc=BeginPaint(h,&p))) return 0;
				BitBlt(dc,0,0,SPECWIDTH,SPECHEIGHT,specdc,0,0,SRCCOPY);
				EndPaint(h,&p);
			}
			return 0;

			//2021sept22, spi, begin
		case WM_MOUSEMOVE:
			mouseTrack.OnMouseMove(h);  // Start tracking.
			xmousepos = GET_X_LPARAM(l); 
			ymousepos = GET_Y_LPARAM(l);
			GetWindowText(h, charbuf, maxchar);
			titlebartext = charbuf;
			found = titlebartext.find(global_windowtitle);
			if (found==std::string::npos)
			{
				//not found! therefore set it
				SetWindowText(h,global_windowtitle.c_str());
			}
			return 0;

		case WM_MOUSELEAVE:
			GetWindowText(h, charbuf, maxchar);
			titlebartext = charbuf;
			found = titlebartext.find(global_filename_tobedisplayed);
			if (found==std::string::npos)
			{
				//not found! therefore set it
				SetWindowText(h,global_filename_tobedisplayed.c_str());
			}
			mouseTrack.Reset(h);
			return 0;

		case WM_MOUSEHOVER:
			GetWindowText(h, charbuf, maxchar);
			titlebartext = charbuf;
			found = titlebartext.find(global_filename_tobedisplayed);
			if (found==std::string::npos)
			{
				//not found! therefore set it
				SetWindowText(h,global_filename_tobedisplayed.c_str());
			}
			mouseTrack.Reset(h);
			return 0;

		case WM_NCMOUSEMOVE:
			GetWindowText(h, charbuf, maxchar);
			titlebartext = charbuf;
			found = titlebartext.find(global_filename_tobedisplayed);
			if (found==std::string::npos)
			{
				//not found! therefore set it
				SetWindowText(h,global_filename_tobedisplayed.c_str());
			}
			return 0;

		case WM_TIMER: 
			if(w==IDT_TIMER_TITLEBAR) 
			{ 
				KillTimer(h, IDT_TIMER_TITLEBAR); 
				SetWindowText(h,global_windowtitle.c_str());
 			} 
			return 0; 
			//2021sept22, spi, end

		case WM_LBUTTONUP:
			if(w==MK_SHIFT)
			{
				float fthreshold = (ymousepos*1.0f)/(SPECHEIGHT+0.0f);
				if (pFILE)
				{
					fprintf(pFILE, "fthreshold = %f\n", fthreshold);
					fflush(pFILE);
				}
				if(fthreshold<0.60f)
				{
					//when mouse click is away from the x-axis (while holding down the shift key)
					//swap mode
					specmode=(specmode+1)%4; // swap spectrum mode
					memset(specbuf,0,SPECWIDTH*SPECHEIGHT);	// clear display
				}
				else
				{
					//when mouse is clicked very close to the x-axis (while holding down the shift key)
					//shift the audio track position
					//QWORD global_length_inbyte
					QWORD offset_inbyte = global_length_inbyte * (xmousepos*1.0f/(SPECWIDTH+0.0f));
					BASS_ChannelSetPosition(chan, offset_inbyte, BASS_POS_BYTE);
					//swap title bar text temporarely
					KillTimer(h, IDT_TIMER_TITLEBAR); 
					SetTimer(h, IDT_TIMER_TITLEBAR, global_windowtitle_notificationtime_ms, NULL);
					double offset_insec = BASS_ChannelBytes2Seconds(chan, offset_inbyte); 
					int offset_hour = offset_insec/3600;
					int offset_min = (offset_insec/60.0f - offset_hour*60.0f);
					int offset_sec = (offset_insec - offset_hour*3600.0f - offset_min*60.0f);
					sprintf(charbuf, "%02d", offset_hour);
					trackposition = charbuf;
					trackposition += "h:";
					sprintf(charbuf, "%02d", offset_min);
					trackposition += charbuf;
					trackposition += "m:";
					sprintf(charbuf, "%02d", offset_sec);
					trackposition += charbuf;
					trackposition += "s";
					trackposition = "Position set to " + trackposition;
					SetWindowText(h,trackposition.c_str());
					if (pFILE)
					{
						fprintf(pFILE, "%s\n", trackposition.c_str());
						fflush(pFILE);
					}
				}
			}
			else
			{	
				//2021sept22, spi, begin
				//change volume
				fvolume = 1.0 - ymousepos/(SPECHEIGHT+0.0f); //0.0;
				BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, fvolume);
				//swap title bar text temporarely
				KillTimer(h, IDT_TIMER_TITLEBAR); 
				SetTimer(h, IDT_TIMER_TITLEBAR, global_windowtitle_notificationtime_ms, NULL);
				int ivolume_percent = fvolume*100;
				sprintf(charbuf, "Volume set to %d%%", ivolume_percent);
				SetWindowText(h,charbuf);
				//2021sept22, spi, end
			}
			return 0;

		case WM_CREATE:
			{
				//2022may14, spi, begin
				SHORT keyState = GetKeyState(VK_SHIFT);
				bool isToggled = keyState & 1;
				bool isDown = keyState & 0x8000;
				if (isDown == true)
				{
					global_bloopmode = true;
					global_fSecondsPlay = 3600.0;
					if (pFILE)
					{
						fprintf(pFILE, "Loopmode, on, %f sec\n", global_fSecondsPlay);
						fflush(pFILE);
					}
				}
				//2022may14, spi, end

				// not playing anything via BASS, so don't need an update thread
				//BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD,0);
				// initialize BASS
				win = h;
				//spi, avril 2015, begin
				SetWindowLong(h, GWL_EXSTYLE, GetWindowLong(h, GWL_EXSTYLE) | WS_EX_LAYERED);
				SetLayeredWindowAttributes(h, 0, global_alpha, LWA_ALPHA);
				//SetLayeredWindowAttributes(h, 0, 200, LWA_ALPHA);
				//spi, avril 2015, end

				//2020oct08, spi, begin
				if (global_isasio == false)
				{
					//if (!BASS_Init(-1, 44100, 0, win, NULL)) //-1 for the default device
					if (!BASS_Init(global_deviceid, 44100, 0, win, NULL)) //-1 for the default device
					{
						Error("Can't initialize device");
						if (pFILE)
						{
							fprintf(pFILE, "Error, cannot initialize device %d\n", global_deviceid);
						}
						return -1;
					}
				}
				else
				{
					if (!BASS_Init(-1, 44100, 0, win, NULL)) //-1 for the default device
					//if(!BASS_Init(global_deviceid,44100,0,win,NULL))
					{
						Error("Can't initialize device");
						if (pFILE)
						{
							fprintf(pFILE, "Error, cannot initialize default device with asio device\n");
						}
						return -1;
					}
				}
				// start a file playing
				if (!PlayFile(global_filename.c_str()))
				{
					if (global_isasio == true) BASS_ASIO_Free();
					BASS_Free();
					return -1;
				}

				//2021sept22, spi, begin
				nmaxchartobedisplayed = min(global_filename.size(), global_windowtitle.size());
				global_filename_tobedisplayed = tail(global_filename, nmaxchartobedisplayed);
				if (global_fvolume < 0.0) global_fvolume = 0.0;
				if (global_fvolume > 1.0) global_fvolume = 1.0;
				BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, global_fvolume);
				global_length_inbyte = BASS_ChannelGetLength(chan, BASS_POS_BYTE); // the length in bytes
				global_length_insec = BASS_ChannelBytes2Seconds(chan, global_length_inbyte); // the length in seconds
				//2021sept22, spi, end

				{ // create bitmap to draw spectrum in (8 bit for easy updating)
					BYTE data[2000] = { 0 };
					BITMAPINFOHEADER *bh = (BITMAPINFOHEADER*)data;
					RGBQUAD *pal = (RGBQUAD*)(data + sizeof(*bh));
					int a;
					bh->biSize = sizeof(*bh);
					bh->biWidth = SPECWIDTH;
					bh->biHeight = SPECHEIGHT; // upside down (line 0=bottom)
					bh->biPlanes = 1;
					bh->biBitCount = 8;
					bh->biClrUsed = bh->biClrImportant = 256;
					/*
					// setup palette, original palette green shift to red
					for (a=1;a<128;a++) {
						pal[a].rgbGreen=256-2*a;
						pal[a].rgbRed=2*a;
					}
					for (a=0;a<32;a++) {
						pal[128+a].rgbBlue=8*a;
						pal[128+32+a].rgbBlue=255;
						pal[128+32+a].rgbRed=8*a;
						pal[128+64+a].rgbRed=255;
						pal[128+64+a].rgbBlue=8*(31-a);
						pal[128+64+a].rgbGreen=8*a;
						pal[128+96+a].rgbRed=255;
						pal[128+96+a].rgbGreen=255;
						pal[128+96+a].rgbBlue=8*a;
					}
					*/

					//altered palette, red shifting to green
					for (a = 1; a < 128; a++) {
						pal[a].rgbRed = 256 - 2 * a;
						pal[a].rgbGreen = 2 * a;
					}
					for (a = 0; a < 32; a++) {
						pal[128 + a].rgbBlue = 8 * a;
						pal[128 + 32 + a].rgbBlue = 255;
						pal[128 + 32 + a].rgbGreen = 8 * a;
						pal[128 + 64 + a].rgbGreen = 255;
						pal[128 + 64 + a].rgbBlue = 8 * (31 - a);
						pal[128 + 64 + a].rgbRed = 8 * a;
						pal[128 + 96 + a].rgbGreen = 255;
						pal[128 + 96 + a].rgbRed = 255;
						pal[128 + 96 + a].rgbBlue = 8 * a;
					}

					/*
					//altered palette, blue shifting to green
					for (a=1;a<128;a++) {
						pal[a].rgbBlue=256-2*a;
						pal[a].rgbGreen=2*a;
					}
					for (a=0;a<32;a++) {
						pal[128+a].rgbBlue=8*a;
						pal[128+32+a].rgbRed=255;
						pal[128+32+a].rgbGreen=8*a;
						pal[128+64+a].rgbGreen=255;
						pal[128+64+a].rgbRed=8*(31-a);
						pal[128+64+a].rgbBlue=8*a;
						pal[128+96+a].rgbGreen=255;
						pal[128+96+a].rgbBlue=255;
						pal[128+96+a].rgbRed=8*a;
					}
					*/
					/*
					//altered palette, black shifting to white - grayscale
					for (a=1;a<256;a++) {
						pal[a].rgbRed=a;
						pal[a].rgbBlue=a;
						pal[a].rgbGreen=a;
					}
					*/
					/*
					//altered palette, solid color
					for (a=1;a<256;a++) {
						pal[a].rgbRed=255;
						pal[a].rgbBlue=100;
						pal[a].rgbGreen=0;
					}
					*/

					// create the bitmap
					specbmp = CreateDIBSection(0, (BITMAPINFO*)bh, DIB_RGB_COLORS, (void**)&specbuf, NULL, 0);
					specdc = CreateCompatibleDC(0);
					SelectObject(specdc, specbmp);
				}
				// setup update timer (40hz)
				timer = timeSetEvent(25, 25, (LPTIMECALLBACK)&UpdateSpectrum, 0, TIME_PERIODIC);
			}
			break;

		case WM_DESTROY:
			{
				//spi, fix, begin
				if((global_isasio==true) && (asiobuf!=NULL)) free(asiobuf);
				//spi, fix, end
				if (timer) timeKillEvent(timer);
				if (global_timer) timeKillEvent(global_timer);
				if(global_isasio==true) BASS_ASIO_Free();
				BASS_Free();
				if (specdc) DeleteDC(specdc);
				if (specbmp) DeleteObject(specbmp);
				if (pFILE)
				{
					fclose(pFILE);
				}
				int nShowCmd = false;
				if(0) ShellExecuteA(NULL, "open", "end.bat", "", NULL, nShowCmd);
				PostQuitMessage(0);
			}
			break;
	}
	return DefWindowProc(h, m, w, l);
}

PCHAR*
    CommandLineToArgvA(
        PCHAR CmdLine,
        int* _argc
        )
    {
        PCHAR* argv;
        PCHAR  _argv;
        ULONG   len;
        ULONG   argc;
        CHAR   a;
        ULONG   i, j;

        BOOLEAN  in_QM;
        BOOLEAN  in_TEXT;
        BOOLEAN  in_SPACE;

        len = strlen(CmdLine);
        i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

        argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
            i + (len+2)*sizeof(CHAR));

        _argv = (PCHAR)(((PUCHAR)argv)+i);

        argc = 0;
        argv[argc] = _argv;
        in_QM = FALSE;
        in_TEXT = FALSE;
        in_SPACE = TRUE;
        i = 0;
        j = 0;

        while( a = CmdLine[i] ) {
            if(in_QM) {
                if(a == '\"') {
                    in_QM = FALSE;
                } else {
                    _argv[j] = a;
                    j++;
                }
            } else {
                switch(a) {
                case '\"':
                    in_QM = TRUE;
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    in_SPACE = FALSE;
                    break;
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if(in_TEXT) {
                        _argv[j] = '\0';
                        j++;
                    }
                    in_TEXT = FALSE;
                    in_SPACE = TRUE;
                    break;
                default:
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    _argv[j] = a;
                    j++;
                    in_SPACE = FALSE;
                    break;
                }
            }
            i++;
        }
        _argv[j] = '\0';
        argv[argc] = NULL;

        (*_argc) = argc;
        return argv;
    }

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nCmdShow)
{
	//2020oct08, spi, begin
	//pFILE = fopen("debug.txt", "w");
	pFILE = NULL;
	//2020oct08, spi, end

	int nShowCmd = false;
	if(0) ShellExecuteA(NULL, "open", "begin.bat", "", NULL, nShowCmd);

	//LPWSTR *szArgList;
	LPSTR *szArgList;
	int argCount;
	//szArgList = CommandLineToArgvW(GetCommandLineW(), &argCount);
	szArgList = CommandLineToArgvA(GetCommandLine(), &argCount);
	if (szArgList == NULL)
	{
		MessageBox(NULL, "Unable to parse command line", "Error", MB_OK);
		if(pFILE) 
		{
			fprintf(pFILE, "Error, unable to parse command line\n");
			fflush(pFILE);
		}
		return 10;
	}
	global_filename="testwav.wav";
	if(argCount>1)
	{
		global_filename = szArgList[1]; 
	}
	global_fSecondsPlay = -1.0; //negative for playing only once
	//global_fSecondsPlay = 30.0f; //negative for playing only once
	if(argCount>2)
	{
		global_fSecondsPlay = atof(szArgList[2]);
	}
	if(argCount>3)
	{
		global_x = atoi(szArgList[3]);
	}
	if(argCount>4)
	{
		global_y = atoi(szArgList[4]);
	}
	if(argCount>5)
	{
		specmode = atoi(szArgList[5]);
	}
	//2022jan08, spi, begin
	//since we are now detecting global_audiodevicename_default
	//global_audiodevicename="Speakers (USB AUDIO  CODEC)"; //"E-MU ASIO"; //"Speakers (2- E-MU E-DSP Audio Processor (WDM))"
	global_audiodevicename = "";
	//2022jan08, spi, end
	if(argCount>6)
	{
		global_audiodevicename = szArgList[6]; //for spi, device name could be "E-MU ASIO", "Speakers (2- E-MU E-DSP Audio Processor (WDM))", etc.
	}
	/*
	global_outputAudioChannelSelectors[0] = 0; // on emu patchmix ASIO device channel 1 (left)
	global_outputAudioChannelSelectors[1] = 1; // on emu patchmix ASIO device channel 2 (right)
	*/
	/*
	global_outputAudioChannelSelectors[0] = 2; // on emu patchmix ASIO device channel 3 (left)
	global_outputAudioChannelSelectors[1] = 3; // on emu patchmix ASIO device channel 4 (right)
	*/
	global_outputAudioChannelSelectors[0] = 6; // on emu patchmix ASIO device channel 15 (left)
	global_outputAudioChannelSelectors[1] = 7; // on emu patchmix ASIO device channel 16 (right)
	if(argCount>7)
	{
		global_outputAudioChannelSelectors[0]=atoi(szArgList[7]); //0 for first asio channel (left) or 2, 4, 6 and 8 for spi (maxed out at 10 asio output channel)
	}
	if(argCount>8)
	{
		global_outputAudioChannelSelectors[1]=atoi(szArgList[8]); //1 for second asio channel (right) or 3, 5, 7 and 9 for spi (maxed out at 10 asio output channel)
	}
	if(argCount>9)
	{
		global_alpha = atoi(szArgList[9]);
	}
	if(argCount>10)
	{
		global_fvolume = atof(szArgList[10]); //could be set to the minimum 0.0, or the maximum 1.0, or in between
	}
	LocalFree(szArgList);

	//2022may14, spi, begin
	SHORT keyState = GetKeyState(VK_SHIFT);
	bool isToggled = keyState & 1;
	bool isDown = keyState & 0x8000;
	if (isDown == true)
	{
		global_bloopmode = true;
		global_fSecondsPlay = 3600.0;
		if (pFILE)
		{
			fprintf(pFILE, "Loopmode, on, %f sec\n", global_fSecondsPlay);
			fflush(pFILE);
		}
	}
	//2022may14, spi, end

	// check the correct BASS was loaded
	if (HIWORD(BASS_GetVersion())!=BASSVERSION) 
	{
		MessageBox(0,"An incorrect version of BASS.DLL was loaded",0,MB_ICONERROR);
		if(pFILE) 
		{
			fprintf(pFILE, "Error, an incorrect version of BASS.DLL was loaded\n");
			fflush(pFILE);
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//ASIO device detection/selection vs non-ASIO default device detection/user specified device selection
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	BASS_ASIO_DEVICEINFO myBASS_ASIO_DEVICEINFO;
	for (int i=0;BASS_ASIO_GetDeviceInfo(i,&myBASS_ASIO_DEVICEINFO);i++)
	{
		string devicenamestring = myBASS_ASIO_DEVICEINFO.name;
		global_asiodevicemap.insert(pair<string, int>(devicenamestring, i));
	}
	//int deviceid=0;
	map<string,int>::iterator it;
	it = global_asiodevicemap.find(global_audiodevicename);
	if(!global_audiodevicename.empty() && it!=global_asiodevicemap.end())
	{
		////////////////////////////////////////////////////////////////////
		//if global_audiodevicename match an installed ASIO device
		////////////////////////////////////////////////////////////////////
		global_deviceid = (*it).second;
		global_isasio = true;
		printf("using installed asio audio device\n");
		printf("%s maps to %d\n", global_audiodevicename.c_str(), global_deviceid);
		if(pFILE) 
		{
			fprintf(pFILE, "using installed asio audio device\n");
			fprintf(pFILE, "%s maps to %d\n", global_audiodevicename.c_str(), global_deviceid);
			fflush(pFILE);
		}
	}
	else
	{
		//2022jan08, spi, begin
		////////////////////////////////////////////////////////////////////
		//if device not specified or does not match an installed asio device
		////////////////////////////////////////////////////////////////////
		global_isasio = false;
		//2022jan08, spi, end
		//
		//2020oct08, spi, begin
		//assert(false);
		//Terminate();
		//now, check if devicename matches an enabled non-asio device
		//note on BASS_GetDeviceInfo(): This function can be used to enumerate the available devices for a setup dialog. Device 0 is always the "no sound" device, so if you should start at device 1 if you only want to list real devices.
		//note on BASS_GetDeviceInfo(): On Linux, a "Default" device is hardcoded to device number 1, which uses the default output set in the ALSA config, and the real devices start at number 2.
		//note on BASS_GetDeviceInfo(): From bass.h header, we see BASS_DEVICEINFO flags:
		//note on BASS_GetDeviceInfo(): #define BASS_DEVICE_ENABLED		1
		//note on BASS_GetDeviceInfo(): #define BASS_DEVICE_DEFAULT		2
		//note on BASS_GetDeviceInfo(): #define BASS_DEVICE_INIT		4
		//note on BASS_GetDeviceInfo(): #define BASS_DEVICE_LOOPBACK	8
		//note on BASS_GetDeviceInfo(): and many more ...
		if (pFILE)
		{
			fprintf(pFILE, "list of installed non-asio audio devices\n");
			fflush(pFILE);
		}
		BASS_DEVICEINFO myBASS_DEVICEINFO;
		for (int i=0;BASS_GetDeviceInfo(i,&myBASS_DEVICEINFO);i++)
		{
			if(myBASS_DEVICEINFO.flags & BASS_DEVICE_ENABLED)
			{
				string devicenamestring = myBASS_DEVICEINFO.name;
				global_nonasiodevicemap.insert(pair<string,int>(devicenamestring,i));
				//2022jan08, spi, begin
				if (myBASS_DEVICEINFO.flags & BASS_DEVICE_DEFAULT)
				{
					global_audiodevicename_default = myBASS_DEVICEINFO.name;
					global_deviceid = i;
					if (pFILE)
					{
						fprintf(pFILE, "%s maps to %d (default non-asio audio device)\n", global_audiodevicename_default.c_str(), global_deviceid);
						fflush(pFILE);
					}
				}
				else
				{
					if (pFILE)
					{
						fprintf(pFILE, "%s maps to %d\n", devicenamestring.c_str(), i);
						fflush(pFILE);
					}
				}
				//2022jan08, spi, end
			}
		}
		//int deviceid=0;
		map<string,int>::iterator it;
		it = global_nonasiodevicemap.find(global_audiodevicename);
		if(!global_audiodevicename.empty() && it!=global_nonasiodevicemap.end())
		{
			global_deviceid = (*it).second;
			//global_isasio = false;
			printf("using user-specified non-asio audio device\n");
			printf("%s maps to %d\n", global_audiodevicename.c_str(), global_deviceid);
			if(pFILE) 
			{
				fprintf(pFILE, "using user-specified non-asio audio device\n");
				fprintf(pFILE, "%s maps to %d\n", global_audiodevicename.c_str(), global_deviceid);
				fflush(pFILE);
			}
		}
		else
		{
			//2022jan08, spi, begin
			it = global_nonasiodevicemap.find(global_audiodevicename_default);
			//if (global_audiodevicename.empty() && !global_audiodevicename_default.empty() && it != global_nonasiodevicemap.end())
			if (!global_audiodevicename_default.empty() && it != global_nonasiodevicemap.end())
			{
				global_deviceid = (*it).second;
				//global_isasio = false;
				printf("using default non-asio audio device\n");
				printf("%s maps to %d\n", global_audiodevicename_default.c_str(), global_deviceid);
				if (pFILE)
				{
					fprintf(pFILE, "using default non-asio audio device\n");
					fprintf(pFILE, "%s maps to %d\n", global_audiodevicename_default.c_str(), global_deviceid);
					fflush(pFILE);
				}
			}
			else
			{
				//assert(false);
				//global_deviceid = 0; //for no sound device (on linux at least)
				global_deviceid = 1; //for first audio device (on linux at least, on windows first device is typically a basic DS or  MME audio device)
				printf("using first non-asio audio device id = %d\n", global_deviceid);
				if (pFILE)
				{
					fprintf(pFILE, "using first non-asio audio device id = %d\n", global_deviceid);
					fflush(pFILE);
				}
			}
			//2022jan08, spi, end
		}
		//2020oct08, spi, end
	}



	WNDCLASS wc={0};
    MSG msg;

	// register window class and create the window
	wc.lpfnWndProc = SpectrumWindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)); //spi, added
	//wc.lpszClassName = "BASS-Spectrum";
	wc.lpszClassName = "spispectrumplay";
	//if (!RegisterClass(&wc) || !CreateWindow("BASS-Spectrum",
	if (!RegisterClass(&wc) || !CreateWindow("spispectrumplay",
			//"BASS spectrum example (click to toggle mode)",
			//"spispectrumplay - click sets vol, shift-click changes mode or pos)",
			global_windowtitle.c_str(),
			//WS_POPUPWINDOW|WS_CAPTION|WS_VISIBLE, 200, 200,
			WS_POPUPWINDOW|WS_CAPTION|WS_VISIBLE, global_x, global_y,
			SPECWIDTH+2*GetSystemMetrics(SM_CXDLGFRAME),
			SPECHEIGHT+GetSystemMetrics(SM_CYCAPTION)+2*GetSystemMetrics(SM_CYDLGFRAME),
			NULL, NULL, hInstance, NULL)) 
	{
		Error("Can't create window");
		if(pFILE) 
		{
			fprintf(pFILE, "Error, can't create window");
			fflush(pFILE);
		}
		return 0;
	}
	ShowWindow(win, SW_SHOWNORMAL);

	while (GetMessage(&msg,NULL,0,0)>0) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
