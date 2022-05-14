function [G] = smith_ggx(L, V, N, r)    
    function [G_] = ggx(V,N,r)
       a  = r.* r;
       a2 = a.* a;
       
       VN = dot(V,N);
       
       den = VN + sqrt((a2 + (1-a2).*(VN.^2)));
       G_ = (2.*VN) ./ den;
    end
    G = ggx(V,N,r).*ggx(L,N,r);
end

