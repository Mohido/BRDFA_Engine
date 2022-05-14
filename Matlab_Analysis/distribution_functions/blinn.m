function [D] = blinn(H, N, r)
    a = r.*r;
    a2 = a.*a;
    cof = 1./(pi.*a2)
    proj = dot(H,N).^(2./a2 - 2)
    D = cof.*proj
end

