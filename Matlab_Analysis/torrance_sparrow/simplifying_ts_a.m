
i_ang = deg2rad(30);
r_ang = deg2rad(-20);
v_i = [cos(i_ang + pi/2) sin(i_ang + pi/2)];
v_r = [cos(r_ang + pi/2) sin(r_ang + pi/2)];
n = [0 1];

theta_max = (i_ang + pi)/3;
theta_shad = 3*i_ang - pi;


%% Drawing the vectors (incident and reflection directions)
p0 = [0 0];
vectarrow(p0,v_i);
hold on
axis equal
vectarrow(p0, v_r);
hold on
axis equal
vectarrow(p0, [0 1]);
hold off

%% Transforming the sin and cosines to dot products.

c_r = dot(v_r, n);      % cos = <a,b>
s2_r = 1-c_r*c_r;       % sin2 = 1 - cos2
s_r = sqrt(s2_r);

disp('testing sins');
disp(sin(r_ang));
disp(s_r);

disp('testing cos');
disp(c_r);
disp(cos(r_ang));

disp('testing cos2((a-b)/2)');
temp = cos((i_ang - r_ang)/2)^2;
disp(temp);
temp = (1/2) * (1 + cos(i_ang - r_ang));
disp(temp);
temp = (1/2) * (1 + dot(v_i,v_r));
disp(temp);


%% Simplifying for A

%{
    Simplifying is done based on transforming the A function into
    its geometric form then simplify it agabriacly. The main rules 
    for transforming the A function from trigonemetric form into 
    algabriac geometric form are:
        1) cos(a) = dot(v_a,v_0)
        2) sin2 = 1 - cos2(a)
        3) sin = sqrt(sin2)
        4) cos(a-b) = dot(a,b)
        5) cos2((a-b)/2) = (1 + dot(a,b))/2
%}

syms x y % x = dot(v_a,v_0) , y = dot(v_a, v_b)
A_simplified = simplify( (1-x*x - (1+y)/2) / ((1+y)/2 - y*(1-x*x)) );


% From the simplification version, we can compare it to the original
% proposed function based on angles.

disp('testing the original "A" function');
A_orig = A_torrance_sparrow_orig(i_ang, r_ang);
disp(A_orig);

disp('testing our "A" function');
A_simp = A_torrance_sparrow_simp(v_i, v_r, n);
disp(A_simp);


%% Testing the Geometric attenuation of both implementations:



