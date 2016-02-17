X = 1000;
xx = [1:X]';
zz = floor(mod((xx/100).^2, 2));
cc = 0.25 + 0.5 * zz;

qclf
qimage(repmat(cc', [100, 1]));

ff = [0:.05:1]; F = length(ff);
cc = repmat(cc, [1 F]);
dd = cc; % Downfiltered version

for f=1:F
  f
  f0 = ff(f);
  z = dd(end,f);
  for x=X-1:-1:1
    z += f0*(dd(x,f)-z);
  end
  for n=1:3
    for x=2:X
      z += f0*(dd(x,f)-z);
      dd(x,f) = z;
    end
    for x=X-1:-1:1
      z += f0*(dd(x,f)-z);
      dd(x,f) = z;
    end
  end
end

qimage(dd')

yy = cc + 0.5 * (cc-dd);

qimage(yy');

yy(yy<.5) = 1 - yy(yy<.5);
plot(ff, mean(yy));
% Shows that effect scales strongly with ff

yy = cc + bsxfun('times', (20/ff), (cc-dd));

qimage(yy');
