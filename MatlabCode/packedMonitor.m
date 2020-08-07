% Test case:
% [freq,fftdB]=packedMonitor(randn(1,2^23),[2^22,2^20,2500,73,100,2]);
function [freqOut,fftOut]=packedMonitor(iqBuffer, settings)
% [sampleNum, NFFT, centerFreq, gain, Fs] = settings{:}; % used for cell
sampleNum = settings(1);
NFFT = settings(2);
centerFreq = settings(3);
gain = settings(4);
Fs = settings(5);
detector = round(settings(6));
fstep = Fs/NFFT;
freq = (-Fs/2:fstep:Fs/2-fstep) + centerFreq;
switch detector
    case 1 %'Sample'
        loopTrip = 1;
    case 2 %'Peak'
        loopTrip = round(sampleNum/NFFT);
    otherwise
        fprintf('Matlab:Unknown Detector:%f',detector);
        return
end
compxDataPool = zeros(loopTrip,NFFT);
%%
scale = 1 / 2^11 / NFFT;
iqBufferDouble = double(iqBuffer(1:loopTrip*NFFT*2));
iqBufferDouble = iqBufferDouble .* repmat(kaiser(NFFT*2,25)',1,loopTrip) * scale;
for ii=1:loopTrip
    iqData = iqBufferDouble((ii-1)*NFFT*2+1:ii*NFFT*2);
    compxDataPool(ii,:) = iqData(1:2:end) + 1j*iqData(2:2:end);
end
fftPool = 20*log10(abs(fftshift(fft(compxDataPool,NFFT,2))))-gain+22; %-51+39-->33dB -51-->73dB
if(loopTrip==1)
    fftdB = fftPool(1,:); 
else
    fftdB = max(fftPool); % find max in each colomn
end
if(NFFT==16384)
    fftOut = fftdB;
    freqOut = freq;
elseif(NFFT<16384)
    fftOut = resample(fftdB, 16384, NFFT);
    fstep = Fs/16384;
    freqOut = (-Fs/2:fstep:Fs/2-fstep) + centerFreq;
else
    switch detector
        case 1 %'Sample'
            fftOut = downsample(fftdB,NFFT/16384);
            freqOut = downsample(freq,NFFT/16384);
        case 2 %'Peak'
            decRate = NFFT/16384;
            fftTmp = zeros(1,16384);
            for ii=1:16384
                fftTmp(ii) = max(fftdB((ii-1)*decRate+1:ii*decRate));
            end
            fftOut = fftTmp;
            freqOut = downsample(freq,NFFT/16384);
        otherwise
            fprintf('Matlab:Unknown Detector:%f',detector);
            return
    end
end