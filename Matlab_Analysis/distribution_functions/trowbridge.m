function [D] = trowbridge(H,N, r)
    a = r.*r;
    a2 = a.*a;
    HN2 = (dot(H,N).^2);
    den = pi.*( HN2 .* (a2 - 1) + 1).^2;
    D = a2 ./ den;
end

