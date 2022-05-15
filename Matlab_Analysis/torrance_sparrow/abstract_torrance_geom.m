function [GF] = abstract_torrance_geom(psi, theta, vectorbased)
%ABSTRACT_TORRANCE_GEOM Summary of this function goes here
%   Detailed explanation goes here
theta_a_n = ( psi - pi)/3;
theta_a_p = ( psi + pi)/3;
theta_aa = 3*psi - pi;

% angles to vectors
v_i = [cos(psi + pi/2) sin(psi + pi/2)];
v_r = [cos(theta + pi/2) sin(theta + pi/2)];
n = [0 1];

cond_1 = (psi <= pi/4);
cond_2 = (pi/4 < psi &&  psi <= pi/2);
if ~vectorbased
    if     cond_1 && ( -pi/2 <= theta && theta <= theta_a_n )
        GF = geometric_attenuation_torrance_sparrow(psi, theta);
    elseif cond_1 && ( theta_a_n <= theta) && (theta <= theta_a_p)
        GF = 1;
    elseif cond_1 && ( theta_a_p <= theta) && (theta <= pi/2)
        GF = geometric_attenuation_torrance_sparrow(psi, theta);
    elseif cond_2 && (-pi/2 <= theta) && (theta <= -psi)
        GF = geometric_attenuation_torrance_sparrow(psi, theta);
    elseif cond_2 && (-psi <= theta) && (theta <= theta_aa)
        GF = geometric_attenuation_torrance_sparrow(theta, psi);
    elseif cond_2 && (theta_aa <= theta) && (theta <= theta_a_p)
        GF = 1;
    else
        GF = geometric_attenuation_torrance_sparrow(psi,theta);
    end
else

    if     cond_1 && ( -pi/2 <= theta && theta <= theta_a_n )
        GF = geometric_attenuation_torrance_sparrow_vector(v_i, v_r, n);
    elseif cond_1 && ( theta_a_n <= theta) && (theta <= theta_a_p)
        GF = 1;
    elseif cond_1 && ( theta_a_p <= theta) && (theta <= pi/2)
        GF = geometric_attenuation_torrance_sparrow_vector(v_i, v_r, n);
    elseif cond_2 && (-pi/2 <= theta) && (theta <= -psi)
        GF = geometric_attenuation_torrance_sparrow_vector(v_i, v_r, n);
    elseif cond_2 && (-psi <= theta) && (theta <= theta_aa)
        GF = geometric_attenuation_torrance_sparrow_vector(v_r, v_i, n);
    elseif cond_2 && (theta_aa <= theta) && (theta <= theta_a_p)
        GF = 1;
    else
        GF = geometric_attenuation_torrance_sparrow_vector(v_i, v_r, n);
    end
    
    
end
end

