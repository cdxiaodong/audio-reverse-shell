# Project Summary

### Summary

**English:**

### Project Description: Audio Receiver

#### Overview
The project is a C++ application designed to receive and decode audio signals transmitted over a specific frequency range. The application captures audio input, processes it to detect predefined tones, and decodes the transmitted data based on these tones. The project is implemented for Windows using the Windows Multimedia API (`winmm.lib`).

#### Key Features
1. **Audio Capture**:
   - The application captures audio input using the Windows Multimedia API (`waveInOpen`, `waveInPrepareHeader`, `waveInAddBuffer`, `waveInStart`).
   - It records audio into a buffer (`bGlobal_AudioBuffer`) with a sample rate of 8000 samples per second and 16 bits per sample.

2. **Tone Detection**:
   - The application uses the Goertzel algorithm to detect specific frequencies in the captured audio samples.
   - It identifies tones corresponding to the start of transmission (`TRANSMISSION_START_FREQUENCY`), data bits (`BIT_TONE_FREQUENCY_ON` for "1" and `BIT_TONE_FREQUENCY_OFF` for "0"), the next bit indicator (`BIT_TONE_FREQUENCY_NEXT`), and the end of transmission (`TRANSMISSION_END_FREQUENCY`).

3. **Data Decoding**:
   - Once a transmission is detected, the application processes the audio samples to identify the strongest tone in each 25ms chunk.
   - It decodes the data bits based on the detected tones and reconstructs the transmitted bytes.
   - The application prints the received bytes to the console.

4. **Error Handling**:
   - The application includes error handling to detect corrupted transmissions and cancels the reception if the detected tones do not meet the expected criteria.

#### Implementation Details
- **Global Variables**:
  - `dwGlobal_TransmissionStarted`: Indicates whether a transmission is in progress.
  - `dwGlobal_BitsReceived`: Tracks the number of bits received.
  - `bGlobal_RecvBits`: Stores the received bits.
  - `dwGlobal_LastToneType`: Stores the type of the last detected tone.
  - `bGlobal_AudioBuffer`: A buffer to store the captured audio samples.

- **Functions**:
  - `CalculateToneMagnitude`: Implements the Goertzel algorithm to calculate the magnitude of a specific frequency in the audio samples.
  - `ProcessSamples`: Processes the captured audio samples to detect and decode the transmitted data.
  - `main`: Initializes the audio capture, sets up the wave format, and continuously captures and processes audio samples.

#### Usage
The application is intended to be run on a Windows system with a microphone input. It listens for specific audio tones that indicate the start, end, and content of a data transmission. Upon detecting a valid transmission, it decodes the data and prints the received bytes to the console.

#### Dependencies
- Windows Multimedia API (`winmm.lib`)
- Standard C and C++ libraries (`stdio.h`, `windows.h`, `math.h`)

#### Potential Enhancements
- Support for different sample rates and bit depths.
- Improved error correction mechanisms.
- Integration with a graphical user interface for better visualization and control.

This project provides a robust foundation for receiving and decoding audio-based data transmissions, with potential applications in various fields such as telecommunications, embedded systems, and audio signal processing.

**Chinese:**

### 项目描述：音频接收器

#### 概述
该项目是一个C++应用程序，旨在接收并解码通过特定频率范围传输的音频信号。该应用程序捕获音频输入，处理以检测预定义的音调，并根据这些音调解码传输的数据。该项目在Windows平台上使用Windows多媒体API（`winmm.lib`）实现。

#### 主要功能
1. **音频捕获**：
   - 应用程序使用Windows多媒体API（`waveInOpen`、`waveInPrepareHeader`、`waveInAddBuffer`、`waveInStart`）捕获音频输入。
   - 它以每秒8000个样本的采样率和每个样本16位的精度将音频记录到缓冲区（`bGlobal_AudioBuffer`）中。

2. **音调检测**：
   - 应用程序使用Goertzel算法检测捕获的音频样本中的特定频率。
   - 它识别与传输开始（`TRANSMISSION_START_FREQUENCY`）、数据位（`BIT_TONE_FREQUENCY_ON`表示“1”，`BIT_TONE_FREQUENCY_OFF`表示“0”）、下一位指示器（`BIT_TONE_FREQUENCY_NEXT`）和传输结束（`TRANSMISSION_END_FREQUENCY`）相对应的音调。

3. **数据解码**：
   - 一旦检测到传输，应用程序处理音频样本以识别每个25毫秒块中最强的音调。
   - 它根据检测到的音调解码数据位，并重建传输的字节。
   - 应用程序将接收到的字节打印到控制台。

4. **错误处理**：
   - 应用程序包括错误处理，以检测损坏的传输，并在检测到的音调不符合预期标准时取消接收。

#### 实现细节
- **全局变量**：
  - `dwGlobal_TransmissionStarted`：指示传输是否正在进行。
  - `dwGlobal_BitsReceived`：跟踪接收到的位数。
  - `bGlobal_RecvBits`：存储接收到的位。
  - `dwGlobal_LastToneType`：存储最后检测到的音调类型。
  - `bGlobal_AudioBuffer`：存储捕获的音频样本的缓冲区。

- **函数**：
  - `CalculateToneMagnitude`：实现Goertzel算法，计算音频样本中特定频率的幅度。
  - `ProcessSamples`：处理捕获的音频样本以检测和解码传输的数据。
  - `main`：初始化音频捕获，设置波形格式，并持续捕获和处理音频样本。

#### 使用方法
该应用程序旨在在具有麦克风输入的Windows系统上运行。它监听指示数据传输开始、结束和内容的特定音频音调。在检测到有效传输后，它解码数据并将接收到的字节打印到控制台。

#### 依赖项
- Windows多媒体API（`winmm.lib`）
- 标准C和C++库（`stdio.h`、`windows.h`、`math.h`）

#### 潜在改进
- 支持不同的采样率和位深度。
- 改进的错误纠正机制。
- 与图形用户界面的集成，以实现更好的可视化和控制。

该项目为接收和解码基于音频的数据传输提供了坚实的基础，具有在电信、嵌入式系统和音频信号处理等领域的潜在应用。

### Content

## File: audio-Receiver.cpp

```
﻿#include <stdio.h>
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
```

----------------------------------------

