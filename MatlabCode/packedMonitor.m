function packedMonitor()
clear all;close all;clc;
% udps = dsp.UDPSender('RemoteIPPort',31000);
udpPackNum = 512;
udpPackLen = 8192;
udpComPackLen = udpPackNum * udpPackLen;
ratio = 512*8192/udpComPackLen;
udpBufferLen = 67108864;
udpBufferPacks = ceil(udpBufferLen / (udpComPackLen*2));% combined packets
fprintf('UDP Buffer Size:  %d Packets\n',udpBufferPacks)
udpr = dsp.UDPReceiver('LocalIPPort',45454,'ReceiveBufferSize',udpBufferLen, ...
    'MessageDataType','int16','MaximumMessageLength',16384); %16384->udpPackLen*2
udpi = dsp.UDPReceiver('LocalIPPort',45456,'MessageDataType','uint8');
setup(udpr); 
setup(udpi);
h=figure('menubar','none','toolbar','none');
set(h,'NumberTitle','off','name','SpaceTY Spectrum Analyzer');
set(h,'Color',[0,0,0]);
drawnow
% hold on
recNum=0;
recPackCnt = 0;
centerFreq=105;
gain=73;
Fs=25;
NFFT= udpComPackLen;
fstep = Fs/NFFT;
freq = -Fs/2:fstep:Fs/2-fstep;
freq = freq + centerFreq;
x = freq;
% iqData = zeros(1,udpComPackLen*2);
iqBuffer = zeros(1,udpComPackLen*20);
fftPool = zeros(udpBufferPacks,NFFT);
exitFlag = 0;

while(1)
    
    
%%
if(exitFlag)
    break
end
ia_uint8 = udpi()';
if(~isempty(ia_uint8))
    ia = char(ia_uint8);
    fprintf('ia: %s\n',ia)
    ia_split = strsplit(ia,';');
    for ii=1:length(ia_split)
        if (isempty(ia_split{1,ii}))
            break %workaround for 1x2cell
        end
        ia_decode = strsplit(ia_split{1,ii},':');
        ia_key = ia_decode{1,1};
        ia_val = ia_decode{1,2};
        switch ia_key
            case 'udpComPackLen'
                udpComPackLen = str2double(ia_val);
%                 fprintf('udpComPackLen: %d\n',udpComPackLen)
            case 'centerFreq'
                centerFreq = str2double(ia_val) / 1e3;
%                 fprintf('centerFreq: %f\n',centerFreq)
            case 'Fs'
                Fs = str2double(ia_val);
%                 fprintf('Fs: %d MHz\n',Fs)
            case 'Gain'
                gain = str2double(ia_val);
%                 fprintf('Gain: %d dB\n',gain)
            case 'Exit'
                exitCode = str2double(ia_val);
                fprintf('Exit:%d\n',ia_val)
                exitFlag = 1;
                break
            case 'clearBuffer'
                release(udpr)
                udpr = dsp.UDPReceiver('LocalIPPort',45454,'ReceiveBufferSize', ...
                    udpBufferLen,'MessageDataType','int16','MaximumMessageLength',16384);
                setup(udpr); 
                recPackCnt = 0;
                fprintf('Buffer Cleared!\n')
            otherwise
                fprintf('Unkonwn Key! %s\n',ia_key);
        end
    end
    
end
NFFT= udpComPackLen;
fstep = Fs/NFFT;
freq = -Fs/2:fstep:Fs/2-fstep;
freq = freq + centerFreq;
x = freq;

%%
tic
while(toc<0.1)
    singlePack = udpr()';
    recNum = recNum + length(singlePack);
    iqBuffer(recNum-length(singlePack)+1:recNum)=singlePack;
end
if(mod((recNum/2),udpComPackLen)~=0)
%     fprintf('Receiving too many packets! Num:%f\n',(recNum/2)/udpComPackLen)
%     break;
    continue
end
loopTrip=(recNum/2)/udpComPackLen;
if(loopTrip>=1)
    for ii=1:loopTrip
    iqData = iqBuffer((ii-1)*udpComPackLen*2+1:ii*udpComPackLen*2);
    % if((recNum/2)==udpComPackLen)
        compxData = double(iqData(1:2:end)) + 1j*double(iqData(2:2:end));
        compxData = compxData .* kaiser(length(compxData),25)';
        compxData = compxData / 2^11 / length(compxData);
        fftPool(ii,1:NFFT) = 20*log10(abs(fftshift(fft(compxData))))-gain+22; %-51+39-->33dB -51-->73dB
    % end
    end
    if(loopTrip==1)
        fftdB = fftPool(1,1:NFFT); 
    else
        fftdB = max(fftPool(1:loopTrip,1:NFFT)); % find max in each colomn
    end
    hold on
    
    patch([min(xlim),max(xlim),max(xlim),min(xlim)], ...
    [min(ylim),min(ylim),max(ylim),max(ylim)],'k')

    plot(freq,fftdB,'Color',[1,0.9,0]);
    hold off

    xlabel('Frequency(MHz)')
    ylabel('Amplitude(dBm)')
    ylim([-160,-20])
    xlim([freq(1),freq(end)])
    grid on
    set(gca,'xcolor',[1,1,1]);
    set(gca,'ycolor',[1,1,1]);
    Ax = gca;
    Ax.Layer = 'top';
    Ax.GridAlpha = 0.5;
    [max_val,index] = max(fftdB);

    txt1 = [num2str(x(index)) 'MHz: ' num2str(max_val) ' dBm'];
    text(centerFreq,-30,txt1,'Color','w')
    drawnow    
    recPackCnt = recPackCnt + recNum/2/udpComPackLen;
%     fprintf('peak:  %f RecPackCnt:     %d\n',max_val, recPackCnt/ratio);
    recNum=0;
end
end

close
release(udpi);
release(udpr);
end