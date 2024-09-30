#include <stdio.h>
#include <windows.h>
#include <math.h>

#pragma comment(lib, "winmm.lib")

#define AUDIO_BITS_PER_SAMPLE 16
#define AUDIO_SAMPLE_RATE 8000

#define TONE_LENGTH_MS 50

#define TRANSMISSION_START_FREQUENCY 2000
#define BIT_TONE_FREQUENCY_ON 600
#define BIT_TONE_FREQUENCY_OFF 800
#define TRANSMISSION_END_FREQUENCY 1500
#define BIT_TONE_FREQUENCY_NEXT 1200

struct WaveHeaderStruct
{
	DWORD dwChunkID;
	DWORD dwChunkSize;
	DWORD dwFormat;

	DWORD dwSubChunk1ID;
	DWORD dwSubChunk1Size;
	WORD wAudioFormat;
	WORD wNumChannels;
	DWORD dwSampleRate;
	DWORD dwByteRate;
	WORD wBlockAlign;
	WORD wBitsPerSample;

	DWORD dwSubChunk2ID;
	DWORD dwSubChunk2Size;
};

FILE* pGlobal_WaveFile = NULL;
DWORD dwGlobal_TotalWaveDataLength = 0;
WaveHeaderStruct Global_WaveHeader;

DWORD InitialiseWaveFile(char* pFilePath)
{
	// create output file
	errno_t err = fopen_s(&pGlobal_WaveFile, pFilePath, "wb");
	if (err != 0 || pGlobal_WaveFile == NULL)
	{
		return 1;
	}

	// reset data length
	dwGlobal_TotalWaveDataLength = 0;

	// generate initial wave header
	memset((void*)&Global_WaveHeader, 0, sizeof(Global_WaveHeader));
	Global_WaveHeader.dwChunkID = 0x46464952;
	Global_WaveHeader.dwChunkSize = 36;
	Global_WaveHeader.dwFormat = 0x45564157;
	Global_WaveHeader.dwSubChunk1ID = 0x20746D66;
	Global_WaveHeader.dwSubChunk1Size = 16;
	Global_WaveHeader.wAudioFormat = 1;
	Global_WaveHeader.wNumChannels = 1;
	Global_WaveHeader.dwSampleRate = AUDIO_SAMPLE_RATE;
	Global_WaveHeader.dwByteRate = AUDIO_SAMPLE_RATE * (AUDIO_BITS_PER_SAMPLE / 8);
	Global_WaveHeader.wBlockAlign = AUDIO_BITS_PER_SAMPLE / 8;
	Global_WaveHeader.wBitsPerSample = AUDIO_BITS_PER_SAMPLE;
	Global_WaveHeader.dwSubChunk2ID = 0x61746164;
	Global_WaveHeader.dwSubChunk2Size = 0;

	// write header to file
	fwrite((void*)&Global_WaveHeader, sizeof(Global_WaveHeader), 1, pGlobal_WaveFile);

	return 0;
}

DWORD CloseWaveFile()
{
	// move back to the start of the file
	rewind(pGlobal_WaveFile);

	// store total data length in wave header
	Global_WaveHeader.dwChunkSize += dwGlobal_TotalWaveDataLength;
	Global_WaveHeader.dwSubChunk2Size += dwGlobal_TotalWaveDataLength;

	// write the updated header
	fwrite((void*)&Global_WaveHeader, sizeof(Global_WaveHeader), 1, pGlobal_WaveFile);

	// close file handle
	fclose(pGlobal_WaveFile);

	return 0;
}

DWORD GenerateTone(DWORD dwFrequency, DWORD dwDuration)
{
	DWORD dwSampleCount = 0;
	DWORD dwTotalSize = 0;
	double dPeriod = 0;
	WORD* pwSampleList = NULL;

	// set initial values
	dwSampleCount = (AUDIO_SAMPLE_RATE * dwDuration) / 1000;
	dwTotalSize = dwSampleCount * sizeof(WORD);
	dPeriod = AUDIO_SAMPLE_RATE / (double)dwFrequency;

	// allocate memory for audio samples
	pwSampleList = (WORD*)malloc(dwTotalSize);
	if (pwSampleList == NULL)
	{
		return 1;
	}

	// generate sine wave in the specified frequency
	for (DWORD i = 0; i < dwSampleCount; i++)
	{
		// store current sample
		pwSampleList[i] = (WORD)(32767 * sin(2 * 3.14159 * ((double)i / dPeriod)));
	}

	// write audio samples to file
	fwrite((void*)pwSampleList, dwTotalSize, 1, pGlobal_WaveFile);

	// increase total length
	dwGlobal_TotalWaveDataLength += dwTotalSize;

	// free temporary memory
	free(pwSampleList);

	return 0;
}

DWORD TransmitByte(BYTE bByte)
{
	DWORD dwCurrBit = 0;
	BYTE bBits[8];

	// convert byte to bits
	dwCurrBit = 128;
	for (DWORD i = 0; i < 8; i++)
	{
		if ((bByte & dwCurrBit) != 0)
		{
			bBits[i] = 1;
		}
		else
		{
			bBits[i] = 0;
		}

		dwCurrBit /= 2;
	}

	// transmit bits
	for (DWORD i = 0; i < 8; i++)
	{
		if (bBits[i] == 0)
		{
			// 0
			if (GenerateTone(BIT_TONE_FREQUENCY_OFF, TONE_LENGTH_MS) != 0)
			{
				return 1;
			}
		}
		else
		{
			// 1
			if (GenerateTone(BIT_TONE_FREQUENCY_ON, TONE_LENGTH_MS) != 0)
			{
				return 1;
			}
		}

		// end of current bit
		if (GenerateTone(BIT_TONE_FREQUENCY_NEXT, TONE_LENGTH_MS) != 0)
		{
			return 1;
		}
	}

	return 0;
}

DWORD TransmitAudioData(BYTE* pData, DWORD dwLength)
{
	char szFileName[512];

	// set temporary filename
	memset(szFileName, 0, sizeof(szFileName));
	strncpy_s(szFileName, "temp_output.wav", sizeof(szFileName) - 1);

	// initialise wave file
	if (InitialiseWaveFile(szFileName) != 0)
	{
		return 1;
	}

	// generate start tone (to indicate start of transmission)
	if (GenerateTone(TRANSMISSION_START_FREQUENCY, TONE_LENGTH_MS) != 0)
	{
		return 1;
	}

	// transmit all bytes
	for (DWORD i = 0; i < dwLength; i++)
	{
		// process current byte
		if (TransmitByte(*(BYTE*)(pData + i)) != 0)
		{
			return 1;
		}
	}

	// generate end tone (to indicate end of transmission)
	if (GenerateTone(TRANSMISSION_END_FREQUENCY, TONE_LENGTH_MS) != 0)
	{
		return 1;
	}

	// close wave file
	CloseWaveFile();

	// play sound
	if (PlaySound((LPCWSTR)szFileName, NULL, SND_SYNC) == 0)
	{
		return 1;
	}

	// delete temporary file
	if (DeleteFile((LPCWSTR)szFileName) == 0)
	{
		return 1;
	}

	return 0;
}

int main(int argc, char* argv[])
{
	BYTE* pData = NULL;
	DWORD dwLength = 0;

	if (argc != 2)
	{
		printf("Usage: %s [data]\n\n", argv[0]);

		return 1;
	}

	// get data and length
	pData = (BYTE*)argv[1];
	dwLength = strlen((char*)pData);

	printf("Sending data...\n");

	// transmit data
	if (TransmitAudioData(pData, dwLength) != 0)
	{
		printf("Error: Failed to send data\n");

		return 1;
	}

	printf("Sent %u bytes successfully\n", dwLength);

	return 0;
}