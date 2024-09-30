#include <stdio.h>
#include <windows.h>
#include <math.h>

#pragma comment(lib, "winmm.lib")

#define AUDIO_BITS_PER_SAMPLE 16
#define AUDIO_SAMPLE_RATE 8000

#define TRANSMISSION_START_FREQUENCY 2000
#define BIT_TONE_FREQUENCY_ON 600
#define BIT_TONE_FREQUENCY_OFF 800
#define TRANSMISSION_END_FREQUENCY 1500
#define BIT_TONE_FREQUENCY_NEXT 1200

#define TONE_THRESHOLD 500

DWORD dwGlobal_TransmissionStarted = 0;
DWORD dwGlobal_BitsReceived = 0;
BYTE bGlobal_RecvBits[8];
DWORD dwGlobal_LastToneType = 0;

// 1 second audio buffer
BYTE bGlobal_AudioBuffer[AUDIO_SAMPLE_RATE * sizeof(WORD)];

double CalculateToneMagnitude(short* pSamples, DWORD dwSampleCount, DWORD dwTargetFrequency)
{
	DWORD dwK = 0;
	double dScalingFactor = 0;
	double dW = 0;
	double dSine = 0;
	double dCosine = 0;
	double dCoeff = 0;
	double dQ0 = 0;
	double dQ1 = 0;
	double dQ2 = 0;
	double dMagnitude = 0;

	// set initial values for goertzel algorithm
	dScalingFactor = (double)dwSampleCount / 2.0;
	dwK = (DWORD)(0.5 + (((double)dwSampleCount * (double)dwTargetFrequency) / AUDIO_SAMPLE_RATE));
	dW = (2.0 * 3.14159 * (double)dwK) / (double)dwSampleCount;
	dSine = sin(dW);
	dCosine = cos(dW);
	dCoeff = 2.0 * dCosine;

	// process all samples
	for (DWORD i = 0; i < dwSampleCount; i++)
	{
		// process current sample
		dQ0 = (dCoeff * dQ1) - dQ2 + pSamples[i];
		dQ2 = dQ1;
		dQ1 = dQ0;
	}

	// calculate magnitude
	dMagnitude = (double)sqrtf((float)((dQ1 * dQ1) + (dQ2 * dQ2) - (dQ1 * dQ2 * dCoeff)));
	dMagnitude /= 100;

	return dMagnitude;
}

DWORD ProcessSamples()
{
	BYTE bRecvByte = 0;
	DWORD dwSamplesRemaining = 0;
	DWORD dwCurrSampleIndex = 0;
	DWORD dwCurrChunkSize = 0;
	short* pCurrSamplePtr = NULL;
	double dFrequencySignal_TransmissionStart = 0;
	double dFrequencySignal_TransmissionStart2 = 0;
	double dFrequencySignal_TransmissionEnd = 0;
	double dFrequencySignal_NextBit = 0;
	double dFrequencySignal_BitOn = 0;
	double dFrequencySignal_BitOff = 0;
	double dStrongestTone = 0;
	DWORD dwStrongestToneType = 0;

	// process current samples
	dwSamplesRemaining = sizeof(bGlobal_AudioBuffer) / sizeof(WORD);
	dwCurrSampleIndex = 0;
	for (;;)
	{
		// check of all samples have been processed
		if (dwSamplesRemaining == 0)
		{
			// finished
			break;
		}

		// calculate current chunk size (25ms)
		dwCurrChunkSize = (AUDIO_SAMPLE_RATE / 40);
		if (dwSamplesRemaining < dwCurrChunkSize)
		{
			dwCurrChunkSize = dwSamplesRemaining;
		}

		// get current sample position
		pCurrSamplePtr = (short*)&bGlobal_AudioBuffer[dwCurrSampleIndex * sizeof(WORD)];

		// check if a transmission is already in progress
		if (dwGlobal_TransmissionStarted == 0)
		{
			// no transmission - check if a new one is starting
			dFrequencySignal_TransmissionStart = CalculateToneMagnitude(pCurrSamplePtr, dwCurrChunkSize, TRANSMISSION_START_FREQUENCY);
			if (dFrequencySignal_TransmissionStart >= TONE_THRESHOLD)
			{
				// new data transmission detected
				dwGlobal_BitsReceived = 0;
				dwGlobal_LastToneType = TRANSMISSION_START_FREQUENCY;
				dwGlobal_TransmissionStarted = 1;
			}
		}
		else
		{
			// a transmission is already in progress - get next tone
			dFrequencySignal_TransmissionStart = CalculateToneMagnitude(pCurrSamplePtr, dwCurrChunkSize, TRANSMISSION_START_FREQUENCY);
			dFrequencySignal_BitOn = CalculateToneMagnitude(pCurrSamplePtr, dwCurrChunkSize, BIT_TONE_FREQUENCY_ON);
			dFrequencySignal_BitOff = CalculateToneMagnitude(pCurrSamplePtr, dwCurrChunkSize, BIT_TONE_FREQUENCY_OFF);
			dFrequencySignal_NextBit = CalculateToneMagnitude(pCurrSamplePtr, dwCurrChunkSize, BIT_TONE_FREQUENCY_NEXT);
			dFrequencySignal_TransmissionEnd = CalculateToneMagnitude(pCurrSamplePtr, dwCurrChunkSize, TRANSMISSION_END_FREQUENCY);

			// check for the strongest tone
			dStrongestTone = 0;
			dwStrongestToneType = 0;
			if (dFrequencySignal_TransmissionStart > dStrongestTone)
			{
				dStrongestTone = dFrequencySignal_TransmissionStart;
				dwStrongestToneType = TRANSMISSION_START_FREQUENCY;
			}
			if (dFrequencySignal_BitOn > dStrongestTone)
			{
				dStrongestTone = dFrequencySignal_BitOn;
				dwStrongestToneType = BIT_TONE_FREQUENCY_ON;
			}
			if (dFrequencySignal_BitOff > dStrongestTone)
			{
				dStrongestTone = dFrequencySignal_BitOff;
				dwStrongestToneType = BIT_TONE_FREQUENCY_OFF;
			}
			if (dFrequencySignal_NextBit > dStrongestTone)
			{
				dStrongestTone = dFrequencySignal_NextBit;
				dwStrongestToneType = BIT_TONE_FREQUENCY_NEXT;
			}
			if (dFrequencySignal_TransmissionEnd > dStrongestTone)
			{
				dStrongestTone = dFrequencySignal_TransmissionEnd;
				dwStrongestToneType = TRANSMISSION_END_FREQUENCY;
			}

			// ensure at least one frequency is above the minimum threshold
			if (dStrongestTone < TONE_THRESHOLD)
			{
				if (dwGlobal_BitsReceived != 0)
				{
					printf("\n** DATA CORRUPT - CANCELLED TRANSMISSION **\n");
				}
				dwGlobal_TransmissionStarted = 0;
			}
			else
			{
				// check if the tone has changed
				if (dwStrongestToneType != dwGlobal_LastToneType)
				{
					// new tone detected
					if (dwStrongestToneType == TRANSMISSION_START_FREQUENCY)
					{
						// found "transmission start" tone but already receiving data
						if (dwGlobal_BitsReceived != 0)
						{
							printf("\n** DATA CORRUPT - CANCELLED TRANSMISSION **\n");
						}

						dwGlobal_TransmissionStarted = 0;
					}
					else if (dwStrongestToneType != BIT_TONE_FREQUENCY_NEXT)
					{
						// check if this is a data bit
						if (dwStrongestToneType == BIT_TONE_FREQUENCY_ON || dwStrongestToneType == BIT_TONE_FREQUENCY_OFF)
						{
							if (dwGlobal_BitsReceived == 0)
							{
								// receiving first data bit
								printf("** RECEIVING DATA **\n");
							}

							// check if the "on" or "off" bit tone is strongest
							if (dwStrongestToneType == BIT_TONE_FREQUENCY_ON)
							{
								// received "1" bit
								bGlobal_RecvBits[7 - (dwGlobal_BitsReceived % 8)] = 1;
							}
							else
							{
								// received "0" bit
								bGlobal_RecvBits[7 - (dwGlobal_BitsReceived % 8)] = 0;
							}

							// wait for confirmation before reading next bit
							dwGlobal_BitsReceived++;

							// check if a full byte (8 bits) has been received
							if (dwGlobal_BitsReceived % 8 == 0)
							{
								// convert bits to byte
								bRecvByte = 0;
								for (DWORD i = 0; i < 8; i++)
								{
									// convert current bit
									bRecvByte |= (bGlobal_RecvBits[i] << i);
								}

								// print current byte
								printf("%c", bRecvByte);
							}
						}
						else if (dwStrongestToneType == TRANSMISSION_END_FREQUENCY)
						{
							// end of transmission
							if (dwGlobal_BitsReceived != 0)
							{
								printf("\n** END OF TRANSMISSION (received %u bytes) **\n", dwGlobal_BitsReceived / 8);
							}
							dwGlobal_TransmissionStarted = 0;
						}
					}

					// store last tone type
					dwGlobal_LastToneType = dwStrongestToneType;
				}
			}
		}

		// update values for next chunk
		dwSamplesRemaining -= dwCurrChunkSize;
		dwCurrSampleIndex += dwCurrChunkSize;
	}

	return 0;
}

int main()
{
	HWAVEIN hWave = NULL;
	WAVEHDR WaveHeaderData;
	WAVEFORMATEX WaveFormatData;
	DWORD dwRetnVal = 0;
	HANDLE hWaveEvent = NULL;


	// create event
	hWaveEvent = CreateEvent(NULL, 1, 0, NULL);
	if (hWaveEvent == NULL)
	{
		return 1;
	}

	// set wave format data
	memset((void*)&WaveFormatData, 0, sizeof(WaveFormatData));
	WaveFormatData.wFormatTag = WAVE_FORMAT_PCM;
	WaveFormatData.wBitsPerSample = AUDIO_BITS_PER_SAMPLE;
	WaveFormatData.nChannels = 1;
	WaveFormatData.nSamplesPerSec = AUDIO_SAMPLE_RATE;
	WaveFormatData.nAvgBytesPerSec = AUDIO_BITS_PER_SAMPLE * (AUDIO_BITS_PER_SAMPLE / 8);
	WaveFormatData.nBlockAlign = AUDIO_BITS_PER_SAMPLE / 8;
	WaveFormatData.cbSize = 0;

	// open wave handle
	if (waveInOpen(&hWave, WAVE_MAPPER, &WaveFormatData, (DWORD)hWaveEvent, 0, CALLBACK_EVENT | WAVE_FORMAT_DIRECT) != MMSYSERR_NOERROR)
	{
		CloseHandle(hWaveEvent);
		return 1;
	}

	for (;;)
	{
		// set wave header data
		memset((void*)&WaveHeaderData, 0, sizeof(WaveHeaderData));
		WaveHeaderData.lpData = (LPSTR)bGlobal_AudioBuffer;
		WaveHeaderData.dwBufferLength = sizeof(bGlobal_AudioBuffer);
		WaveHeaderData.dwBytesRecorded = 0;
		WaveHeaderData.dwUser = 0;
		WaveHeaderData.dwFlags = 0;
		WaveHeaderData.dwLoops = 0;

		// prepare wave header
		if (waveInPrepareHeader(hWave, &WaveHeaderData, sizeof(WaveHeaderData)) != MMSYSERR_NOERROR)
		{
			// error
			CloseHandle(hWaveEvent);
			waveInClose(hWave);

			return 1;
		}

		// add wave input buffer
		if (waveInAddBuffer(hWave, &WaveHeaderData, sizeof(WaveHeaderData)) != MMSYSERR_NOERROR)
		{
			// error
			CloseHandle(hWaveEvent);
			waveInClose(hWave);

			return 1;
		}

		// reset event
		ResetEvent(hWaveEvent);

		// start recording
		if (waveInStart(hWave) != MMSYSERR_NOERROR)
		{
			// error
			CloseHandle(hWaveEvent);
			waveInClose(hWave);

			return 1;
		}

		// wait until recording has finished
		for (;;)
		{
			// wait for event to fire
			WaitForSingleObject(hWaveEvent, INFINITE);

			// check if sample has finished recording
			if (WaveHeaderData.dwFlags & WHDR_DONE)
			{
				// finished
				break;
			}
		}

		// unprepare wave header
		if (waveInUnprepareHeader(hWave, &WaveHeaderData, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			// error
			CloseHandle(hWaveEvent);
			waveInClose(hWave);

			return 1;
		}

		// process audio samples
		if (ProcessSamples() != 0)
		{
			// error
			CloseHandle(hWaveEvent);
			waveInClose(hWave);

			return 1;
		}
	}

	// close wave handle
	waveInClose(hWave);

	// close event
	CloseHandle(hWaveEvent);

	return 0;
}