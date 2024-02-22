module StdMath
    // create extern functions for all math functions from math.h
    public extern function sqrt(in real x): real
    end sqrt;
    public extern function exp(in real x): real
    end exp;
    public extern function log(in real x): real
    end log;
    public extern function log2(in real x): real
    end log2;
    public extern function log10(in real x): real
    end log10;
    public extern function pow(in real x, in real y): real
    end pow;
    public extern function sinh(in real x): real
    end sinh;
    public extern function cosh(in real x): real
    end cosh;
    public extern function tanh(in real x): real
    end tanh;
    public extern function asin(in real x): real
    end asin;
    public extern function acos(in real x): real
    end acos;
    public extern function atan(in real x): real
    end atan;
    public extern function atan2(in real x, in real y): real
    end atan2;
    public extern function sin(in real x): real
    end sin;
    public extern function cos(in real x): real
    end cos;
    public extern function tan(in real x): real
    end tan;
    public extern function asinh(in real x): real
    end asinh;
    public extern function acosh(in real x): real
    end acosh;
    public extern function atanh(in real x): real
    end atanh;
    public extern function hypot(in real x, in real y): real
    end hypot;
    public extern function cbrt(in real x): real
    end cbrt;
    public extern function ceil(in real x): real
    end ceil;
    public extern function floor(in real x): real
    end floor;
    public extern function trunc(in real x): real
    end trunc;
    public extern function round(in real x): real
    end round;
    public extern function ldexp(in real x, in integer y): real
    end ldexp;
    public extern function frexp(in real x, out integer y): real
    end frexp;
    public extern function modf(in real x, out real y): real
    end modf;
    public extern function fmod(in real x, in real y): real
    end fmod;
    public extern function copysign(in real x, in real y): real
    end copysign;
    public extern function nextafter(in real x, in real y): real
    end nextafter;
    public extern function nexttoward(in real x, in real y): real
    end nexttoward;
    public extern function fdim(in real x, in real y): real
    end fdim;
    public extern function fmax(in real x, in real y): real
    end fmax;
    public extern function fmin(in real x, in real y): real
    end fmin;
    public extern function fma(in real x, in real y, in real z): real
    end fma;
    public extern function isnan(in real x): boolean
    end isnan;
    public extern function signbit(in real x): boolean
    end signbit;
    public extern function fabs(in real x): real
    end fabs;
    public extern function abs(in integer x): integer
    end abs;
start
    output sin(5.0);
    // create test for every function
    output sqrt(5.0);
    output exp(5.0);
    output log(5.0);
    output log2(5.0);
    output log10(5.0);
    output pow(5.0, 2.0);
    output sinh(5.0);
    output cosh(5.0);
    output tanh(5.0);
    output asin(5.0);
    output acos(5.0);
    output atan(5.0);
    output atan2(5.0, 2.0);
    output sin(5.0);
    output cos(5.0);
    output tan(5.0);
    output asinh(5.0);
    output acosh(5.0);
    output atanh(5.0);
    output hypot(5.0, 2.0);
    output cbrt(5.0);
    output ceil(5.0);
    output floor(5.0);
    output trunc(5.0);
    output round(5.0);
    output ldexp(5.0, 2);
    output fmod(5.0, 2.0);
    output copysign(5.0, 2.0);
    output nextafter(5.0, 2.0);
    output nexttoward(5.0, 2.0);
    output fdim(5.0, 2.0);
    output fmax(5.0, 2.0);
    output fmin(5.0, 2.0);
    output fma(5.0, 2.0, 3.0);
    output isnan(5.0);
    output signbit(5.0);
    output fabs(5.0);
end StdMath.