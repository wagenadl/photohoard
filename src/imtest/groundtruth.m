img = [ 0 0 0; 1 1 1; .5 .5 .5; .6 .4 .4; .6 .6 .4; .4 .4 .6];
nam = strtoks('Black White Gray Red Yellow Blue');
img = reshape(img, [1 6 3]);
imwrite(img, 'img.png');

srgb0 = img;
rgb = colorconvert(srgb0, 'from', 'srgb', 'to', 'linearrgb');
xyz = colorconvert(srgb0, 'from', 'srgb', 'to', 'ciexyz');
lab = colorconvert(srgb0, 'from', 'srgb', 'to', 'cielab');
lch = colorconvert(srgb0, 'from', 'srgb', 'to', 'cielch');
srgb1 = colorconvert(lch, 'from', 'cielch', 'to', 'srgb');

all = {srgb0, rgb, xyz, lab, lch, srgb1 };
lbl = strtoks('sRGB RGB XYZ Lab Lch sRGB');
K = length(all);
for k=1:K
  printf('%s:\n', lbl{k});
  for n=1:6
    printf('  ');
%  printf('%s:\n', nam{n});
    for c=1:3
      printf('%12s', num2thou(all{k}(1,n,c),6));
    end
  printf('\n');
  end
  printf('\n');
end
