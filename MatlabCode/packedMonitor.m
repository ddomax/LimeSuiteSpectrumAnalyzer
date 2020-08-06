function packedMonitor(iqBuffer, sampleNum, NFFT)
tic
udpComPackLen = sampleNum;
% h=figure('menubar','none','toolbar','none');
% set(h,'NumberTitle','off','name','SpaceTY Spectrum Analyzer');
% set(h,'Color',[0,0,0]);
% drawnow
centerFreq=105;
gain=73;
Fs=25;
fstep = Fs/NFFT;
freq = -Fs/2:fstep:Fs/2-fstep;
freq = freq + centerFreq;
x = freq;
% iqData = zeros(1,udpComPackLen*2);
% iqBuffer = zeros(1,udpComPackLen*20);
udpBufferPacks = round(udpComPackLen/NFFT);
% fftPool = zeros(udpBufferPacks,NFFT);
compxDataPool = zeros(udpBufferPacks,NFFT);

%%
loopTrip=udpBufferPacks;
scale = 1 / 2^11 / NFFT;
if(loopTrip>=1)
    iqBufferDouble = double(iqBuffer(1:sampleNum*2));
    iqBufferDouble = iqBufferDouble .* repmat(kaiser(NFFT*2,25)',1,loopTrip) * scale;
    for ii=1:loopTrip
        iqData = iqBufferDouble((ii-1)*NFFT*2+1:ii*NFFT*2);
%         iqData = iqData .* kaiser(NFFT*2,25)' / 2^11 / NFFT;
        compxDataPool(ii,:) = iqData(1:2:end) + 1j*iqData(2:2:end);
    end
%     for ii=1:loopTrip
%         compxData = compxDataPool(ii,:);
%         compxData = compxData .* kaiser(length(compxData),25)';
%         compxData = compxData / 2^11 / length(compxData);
%         fftPool(ii,:) = 20*log10(abs(fftshift(fft(compxData))))-gain+22; %-51+39-->33dB -51-->73dB
%     end
    fftPool = 20*log10(abs(fftshift(fft(compxDataPool,NFFT,2))))-gain+22; %-51+39-->33dB -51-->73dB
    if(loopTrip==1)
        fftdB = fftPool(1,:); 
    else
        fftdB = max(fftPool); % find max in each colomn
    end
%     hold on
%     
%     patch([min(xlim),max(xlim),max(xlim),min(xlim)], ...
%     [min(ylim),min(ylim),max(ylim),max(ylim)],'k')

%     plot(freq,fftdB,'Color',[1,0.9,0]);
%     freq_dec = downsample(freq,NFFT/2^10);
%     fftdB_dec = downsample(fftdB,NFFT/2^10);
    plot(freq,fftdB)
%     hold off

    xlabel('Frequency(MHz)')
    ylabel('Amplitude(dBm)')
    ylim([-160,-20])
    xlim([freq(1),freq(end)])
    grid on
%     set(gca,'xcolor',[1,1,1]);
%     set(gca,'ycolor',[1,1,1]);
%     Ax = gca;
%     Ax.Layer = 'top';
%     Ax.GridAlpha = 0.5;
%     [max_val,index] = max(fftdB);

%     txt1 = [num2str(x(index)) 'MHz: ' num2str(max_val) ' dBm'];
%     text(centerFreq,-30,txt1,'Color','w')
    drawnow   
%     fprintf('%6.2f MHz %5.2f dBm\n',freq(index), max_val);
end
% close
fprintf('toc:%f\n',toc)
end