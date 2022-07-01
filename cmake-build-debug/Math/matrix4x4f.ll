; ModuleID = 'matrix4x4f'
source_filename = "matrix4x4f"
target triple = "x86_64-w64-windows-gnu"

%StdString.String = type { i8*, i32 }
%matrix4x4f.Matrix4x4f = type { float** }

@0 = private unnamed_addr constant [8 x i8] c"hello22\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@3 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@4 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@5 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@6 = private unnamed_addr constant [4 x i8] c"%c\0A\00", align 1
@7 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@8 = private unnamed_addr constant [5 x i8] c"~~~~\00", align 1
@9 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@10 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1

declare i32 @printf(i8*, ...)

; Function Attrs: cold noreturn nounwind
declare void @llvm.trap() #0

declare void @GC_init()

declare i8* @GC_malloc(i32)

declare void @GC_free(i8*)

declare double @sin(double)

declare double @cos(double)

declare double @sqrt(double)

declare double @exp(double)

declare double @log(double)

declare double @log2(double)

declare double @log10(double)

declare double @pow(double, double)

declare double @sinh(double)

declare double @cosh(double)

declare double @tanh(double)

declare double @asin(double)

declare double @acos(double)

declare double @atan(double)

declare double @atan2(double, double)

declare double @sin.1(double)

declare double @cos.2(double)

declare double @tan(double)

declare double @asinh(double)

declare double @acosh(double)

declare double @atanh(double)

declare double @hypot(double, double)

declare double @cbrt(double)

declare double @ceil(double)

declare double @floor(double)

declare double @trunc(double)

declare double @round(double)

declare double @ldexp(double, i32)

declare double @frexp(double, i32*)

declare double @modf(double, double*)

declare double @fmod(double, double)

declare double @copysign(double, double)

declare double @nextafter(double, double)

declare double @nexttoward(double, double)

declare double @fdim(double, double)

declare double @fmax(double, double)

declare double @fmin(double, double)

declare double @fma(double, double, double)

declare i1 @isnan(double)

declare i1 @signbit(double)

declare double @fabs(double)

declare i32 @"StdString.strleni8*"(i8*)

declare i8* @"StdString.strcpyi8*i8*"(i8*, i8*)

declare i8* @"StdString.strcati8*i8*"(i8*, i8*)

declare i32 @"StdString.strstri8*i8*"(i8*, i8*)

declare i32 @"StdString.strstri8*i8*i32"(i8*, i8*, i32)

declare void @StdString.String._DefaultConstructor_(%StdString.String*)

declare void @"StdString.String.clear%StdString.String*"(%StdString.String*)

declare void @"StdString.String.init%StdString.String*i8*"(%StdString.String*, i8*)

declare void @"StdString.String.concat%StdString.String*i8*"(%StdString.String*, i8*)

declare %StdString.String* @"StdString.String.substr%StdString.String*i32i32"(%StdString.String*, i32, i32)

declare %StdString.String* @"StdString.String.substr%StdString.String*i32"(%StdString.String*, i32)

declare i32 @"StdString.String.find%StdString.String*i8*"(%StdString.String*, i8*)

declare %StdString.String* @"StdString.String.replace%StdString.String*i8i8"(%StdString.String*, i8, i8)

declare %StdString.String* @"StdString.String.insert%StdString.String*i32i8"(%StdString.String*, i32, i8)

declare %StdString.String* @"StdString.String.replaceAll%StdString.String*i8*i8*"(%StdString.String*, i8*, i8*)

declare i32 @"StdString.String.len%StdString.String*"(%StdString.String*)

declare i8* @"StdString.String.toString%StdString.String*"(%StdString.String*)

declare i8 @"StdString.String.getSymbol%StdString.String*i32"(%StdString.String*, i32)

declare void @"StdString.String.setSymbol%StdString.String*i32i8"(%StdString.String*, i32, i8)

declare i8* @StdString.IntToCharArrayi32(i32)

declare %StdString.String* @StdString.IntToStringi32(i32)

declare i8* @StdString.RealToCharArraydoublei32(double, i32)

declare %StdString.String* @StdString.RealToStringdoublei32(double, i32)

define void @matrix4x4f.Matrix4x4f._DefaultConstructor_(%matrix4x4f.Matrix4x4f* %0) {
matrix4x4f.Matrix4x4f_DefaultConstructor_Entry:
  %1 = alloca %matrix4x4f.Matrix4x4f*, align 8
  store %matrix4x4f.Matrix4x4f* %0, %matrix4x4f.Matrix4x4f** %1, align 8
  %2 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %1, align 8
  %3 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %2, i32 0, i32 0
  %4 = mul i32 8, 4
  %malloccall = tail call i8* @malloc(i32 %4)
  %5 = bitcast i8* %malloccall to float**
  store float** %5, float*** %3, align 8
  %j0 = alloca i32, align 4
  store i32 0, i32* %j0, align 4
  %j1 = alloca i32, align 4
  store i32 0, i32* %j1, align 4
  br label %6

6:                                                ; preds = %8, %matrix4x4f.Matrix4x4f_DefaultConstructor_Entry
  %7 = load i32, i32* %j1, align 4
  %"j<sz" = icmp slt i32 %7, 4
  br i1 %"j<sz", label %8, label %19

8:                                                ; preds = %6
  %9 = load i32, i32* %j1, align 4
  %10 = mul i32 4, 4
  %malloccall1 = tail call i8* @malloc(i32 %10)
  %11 = bitcast i8* %malloccall1 to float*
  %12 = load float**, float*** %3, align 8
  %13 = load i32, i32* %j1, align 4
  %14 = sext i32 %13 to i64
  %15 = getelementptr inbounds float*, float** %12, i64 %14
  %16 = load float*, float** %15, align 8
  store float* %11, float** %15, align 8
  %17 = load i32, i32* %j1, align 4
  %18 = add i32 %17, 1
  store i32 %18, i32* %j1, align 4
  br label %6

19:                                               ; preds = %6
  ret void
}

declare noalias i8* @malloc(i32)

define i8* @"matrix4x4f.Matrix4x4f.toString%matrix4x4f.Matrix4x4f*"(%matrix4x4f.Matrix4x4f* %varthis) {
matrix4x4fmatrix4x4f.Matrix4x4f.toStringEntry:
  %this = alloca %matrix4x4f.Matrix4x4f*, align 8
  store %matrix4x4f.Matrix4x4f* %varthis, %matrix4x4f.Matrix4x4f** %this, align 8
  call void @llvm.trap()
  unreachable
}

define void @"matrix4x4f.Matrix4x4f.init%matrix4x4f.Matrix4x4f*float"(%matrix4x4f.Matrix4x4f* %outthis, float %inn) {
matrix4x4fmatrix4x4f.Matrix4x4f.initEntry:
  %this = alloca %matrix4x4f.Matrix4x4f*, align 8
  store %matrix4x4f.Matrix4x4f* %outthis, %matrix4x4f.Matrix4x4f** %this, align 8
  %n = alloca float, align 4
  store float %inn, float* %n, align 4
  %0 = load float, float* %n, align 4
  %1 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %this, align 8
  %2 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %1, i32 0, i32 0
  %3 = load float**, float*** %2, align 8
  %4 = getelementptr inbounds float*, float** %3, i32 0
  %5 = load float*, float** %4, align 8
  %6 = getelementptr inbounds float, float* %5, i32 0
  %7 = load float, float* %6, align 4
  store float %0, float* %6, align 4
  %8 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %this, align 8
  %9 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %8, i32 0, i32 0
  %10 = load float**, float*** %9, align 8
  %11 = getelementptr inbounds float*, float** %10, i32 0
  %12 = load float*, float** %11, align 8
  %13 = getelementptr inbounds float, float* %12, i32 1
  %14 = load float, float* %13, align 4
  store float 0.000000e+00, float* %13, align 4
  %15 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %this, align 8
  %16 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %15, i32 0, i32 0
  %17 = load float**, float*** %16, align 8
  %18 = getelementptr inbounds float*, float** %17, i32 0
  %19 = load float*, float** %18, align 8
  %20 = getelementptr inbounds float, float* %19, i32 2
  %21 = load float, float* %20, align 4
  store float 0.000000e+00, float* %20, align 4
  %22 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %this, align 8
  %23 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %22, i32 0, i32 0
  %24 = load float**, float*** %23, align 8
  %25 = getelementptr inbounds float*, float** %24, i32 0
  %26 = load float*, float** %25, align 8
  %27 = getelementptr inbounds float, float* %26, i32 3
  %28 = load float, float* %27, align 4
  store float 0.000000e+00, float* %27, align 4
  %29 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %this, align 8
  %30 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %29, i32 0, i32 0
  %31 = load float**, float*** %30, align 8
  %32 = getelementptr inbounds float*, float** %31, i32 1
  %33 = load float*, float** %32, align 8
  %34 = getelementptr inbounds float, float* %33, i32 0
  %35 = load float, float* %34, align 4
  store float 0.000000e+00, float* %34, align 4
  %36 = load float, float* %n, align 4
  %37 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %this, align 8
  %38 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %37, i32 0, i32 0
  %39 = load float**, float*** %38, align 8
  %40 = getelementptr inbounds float*, float** %39, i32 1
  %41 = load float*, float** %40, align 8
  %42 = getelementptr inbounds float, float* %41, i32 1
  %43 = load float, float* %42, align 4
  store float %36, float* %42, align 4
  %44 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %this, align 8
  %45 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %44, i32 0, i32 0
  %46 = load float**, float*** %45, align 8
  %47 = getelementptr inbounds float*, float** %46, i32 1
  %48 = load float*, float** %47, align 8
  %49 = getelementptr inbounds float, float* %48, i32 2
  %50 = load float, float* %49, align 4
  store float 0.000000e+00, float* %49, align 4
  %51 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %this, align 8
  %52 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %51, i32 0, i32 0
  %53 = load float**, float*** %52, align 8
  %54 = getelementptr inbounds float*, float** %53, i32 1
  %55 = load float*, float** %54, align 8
  %56 = getelementptr inbounds float, float* %55, i32 3
  %57 = load float, float* %56, align 4
  store float 0.000000e+00, float* %56, align 4
  %58 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %this, align 8
  %59 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %58, i32 0, i32 0
  %60 = load float**, float*** %59, align 8
  %61 = getelementptr inbounds float*, float** %60, i32 2
  %62 = load float*, float** %61, align 8
  %63 = getelementptr inbounds float, float* %62, i32 0
  %64 = load float, float* %63, align 4
  store float 0.000000e+00, float* %63, align 4
  %65 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %this, align 8
  %66 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %65, i32 0, i32 0
  %67 = load float**, float*** %66, align 8
  %68 = getelementptr inbounds float*, float** %67, i32 2
  %69 = load float*, float** %68, align 8
  %70 = getelementptr inbounds float, float* %69, i32 1
  %71 = load float, float* %70, align 4
  store float 0.000000e+00, float* %70, align 4
  %72 = load float, float* %n, align 4
  %73 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %this, align 8
  %74 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %73, i32 0, i32 0
  %75 = load float**, float*** %74, align 8
  %76 = getelementptr inbounds float*, float** %75, i32 2
  %77 = load float*, float** %76, align 8
  %78 = getelementptr inbounds float, float* %77, i32 2
  %79 = load float, float* %78, align 4
  store float %72, float* %78, align 4
  %80 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %this, align 8
  %81 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %80, i32 0, i32 0
  %82 = load float**, float*** %81, align 8
  %83 = getelementptr inbounds float*, float** %82, i32 2
  %84 = load float*, float** %83, align 8
  %85 = getelementptr inbounds float, float* %84, i32 3
  %86 = load float, float* %85, align 4
  store float 0.000000e+00, float* %85, align 4
  %87 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %this, align 8
  %88 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %87, i32 0, i32 0
  %89 = load float**, float*** %88, align 8
  %90 = getelementptr inbounds float*, float** %89, i32 3
  %91 = load float*, float** %90, align 8
  %92 = getelementptr inbounds float, float* %91, i32 0
  %93 = load float, float* %92, align 4
  store float 0.000000e+00, float* %92, align 4
  %94 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %this, align 8
  %95 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %94, i32 0, i32 0
  %96 = load float**, float*** %95, align 8
  %97 = getelementptr inbounds float*, float** %96, i32 3
  %98 = load float*, float** %97, align 8
  %99 = getelementptr inbounds float, float* %98, i32 1
  %100 = load float, float* %99, align 4
  store float 0.000000e+00, float* %99, align 4
  %101 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %this, align 8
  %102 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %101, i32 0, i32 0
  %103 = load float**, float*** %102, align 8
  %104 = getelementptr inbounds float*, float** %103, i32 3
  %105 = load float*, float** %104, align 8
  %106 = getelementptr inbounds float, float* %105, i32 2
  %107 = load float, float* %106, align 4
  store float 0.000000e+00, float* %106, align 4
  %108 = load float, float* %n, align 4
  %109 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %this, align 8
  %110 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %109, i32 0, i32 0
  %111 = load float**, float*** %110, align 8
  %112 = getelementptr inbounds float*, float** %111, i32 3
  %113 = load float*, float** %112, align 8
  %114 = getelementptr inbounds float, float* %113, i32 3
  %115 = load float, float* %114, align 4
  store float %108, float* %114, align 4
  ret void
}

define float** @"matrix4x4f.Matrix4x4f.rawData%matrix4x4f.Matrix4x4f*"(%matrix4x4f.Matrix4x4f* %outthis) {
matrix4x4fmatrix4x4f.Matrix4x4f.rawDataEntry:
  %this = alloca %matrix4x4f.Matrix4x4f*, align 8
  store %matrix4x4f.Matrix4x4f* %outthis, %matrix4x4f.Matrix4x4f** %this, align 8
  %0 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %this, align 8
  %1 = getelementptr inbounds %matrix4x4f.Matrix4x4f, %matrix4x4f.Matrix4x4f* %0, i32 0, i32 0
  %2 = load float**, float*** %1, align 8
  ret float** %2
}

define %StdString.String** @"matrix4x4f.Matrix4x4f.test%matrix4x4f.Matrix4x4f*"(%matrix4x4f.Matrix4x4f* %outthis) {
matrix4x4fmatrix4x4f.Matrix4x4f.testEntry:
  %this = alloca %matrix4x4f.Matrix4x4f*, align 8
  store %matrix4x4f.Matrix4x4f* %outthis, %matrix4x4f.Matrix4x4f** %this, align 8
  %aaa = alloca %StdString.String**, align 8
  %0 = mul i32 8, 4
  %malloccall = tail call i8* @malloc(i32 %0)
  %1 = bitcast i8* %malloccall to %StdString.String**
  store %StdString.String** %1, %StdString.String*** %aaa, align 8
  %j0 = alloca i32, align 4
  store i32 0, i32* %j0, align 4
  %j1 = alloca i32, align 4
  store i32 0, i32* %j1, align 4
  br label %2

2:                                                ; preds = %4, %matrix4x4fmatrix4x4f.Matrix4x4f.testEntry
  %3 = load i32, i32* %j1, align 4
  %"j<sz" = icmp slt i32 %3, 4
  br i1 %"j<sz", label %4, label %15

4:                                                ; preds = %2
  %5 = load i32, i32* %j1, align 4
  %6 = mul i32 4, 16
  %malloccall1 = tail call i8* @malloc(i32 %6)
  %7 = bitcast i8* %malloccall1 to %StdString.String*
  %8 = load %StdString.String**, %StdString.String*** %aaa, align 8
  %9 = load i32, i32* %j1, align 4
  %10 = sext i32 %9 to i64
  %11 = getelementptr inbounds %StdString.String*, %StdString.String** %8, i64 %10
  %12 = load %StdString.String*, %StdString.String** %11, align 8
  store %StdString.String* %7, %StdString.String** %11, align 8
  %13 = load i32, i32* %j1, align 4
  %14 = add i32 %13, 1
  store i32 %14, i32* %j1, align 4
  br label %2

15:                                               ; preds = %2
  %16 = load %StdString.String**, %StdString.String*** %aaa, align 8
  %17 = getelementptr inbounds %StdString.String*, %StdString.String** %16, i32 2
  %18 = load %StdString.String*, %StdString.String** %17, align 8
  %19 = getelementptr inbounds %StdString.String, %StdString.String* %18, i32 1
  %20 = load %StdString.String, %StdString.String* %19, align 8
  call void @"StdString.String.init%StdString.String*i8*"(%StdString.String* %19, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @0, i32 0, i32 0))
  %21 = load %StdString.String**, %StdString.String*** %aaa, align 8
  ret %StdString.String** %21
}

define i32 @main() {
entry:
  call void @GC_init()
  %m = alloca %matrix4x4f.Matrix4x4f*, align 8
  %malloccall = tail call i8* @malloc(i32 8)
  %0 = bitcast i8* %malloccall to %matrix4x4f.Matrix4x4f*
  store %matrix4x4f.Matrix4x4f* %0, %matrix4x4f.Matrix4x4f** %m, align 8
  %1 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %m, align 8
  call void @matrix4x4f.Matrix4x4f._DefaultConstructor_(%matrix4x4f.Matrix4x4f* %1)
  %2 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %m, align 8
  call void @"matrix4x4f.Matrix4x4f.init%matrix4x4f.Matrix4x4f*float"(%matrix4x4f.Matrix4x4f* %2, float 1.000000e+00)
  %dat = alloca float**, align 8
  %3 = mul i32 8, 4
  %malloccall1 = tail call i8* @malloc(i32 %3)
  %4 = bitcast i8* %malloccall1 to float**
  store float** %4, float*** %dat, align 8
  %j0 = alloca i32, align 4
  store i32 0, i32* %j0, align 4
  %j1 = alloca i32, align 4
  store i32 0, i32* %j1, align 4
  br label %5

5:                                                ; preds = %7, %entry
  %6 = load i32, i32* %j1, align 4
  %"j<sz" = icmp slt i32 %6, 4
  br i1 %"j<sz", label %7, label %18

7:                                                ; preds = %5
  %8 = load i32, i32* %j1, align 4
  %9 = mul i32 4, 4
  %malloccall2 = tail call i8* @malloc(i32 %9)
  %10 = bitcast i8* %malloccall2 to float*
  %11 = load float**, float*** %dat, align 8
  %12 = load i32, i32* %j1, align 4
  %13 = sext i32 %12 to i64
  %14 = getelementptr inbounds float*, float** %11, i64 %13
  %15 = load float*, float** %14, align 8
  store float* %10, float** %14, align 8
  %16 = load i32, i32* %j1, align 4
  %17 = add i32 %16, 1
  store i32 %17, i32* %j1, align 4
  br label %5

18:                                               ; preds = %5
  %19 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %m, align 8
  %20 = call float** @"matrix4x4f.Matrix4x4f.rawData%matrix4x4f.Matrix4x4f*"(%matrix4x4f.Matrix4x4f* %19)
  store float** %20, float*** %dat, align 8
  %21 = load float**, float*** %dat, align 8
  %22 = getelementptr inbounds float*, float** %21, i32 0
  %23 = load float*, float** %22, align 8
  %24 = getelementptr inbounds float, float* %23, i32 0
  %25 = load float, float* %24, align 4
  %26 = load float, float* %24, align 4
  %27 = fpext float %26 to double
  %28 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @1, i32 0, i32 0), double %27)
  %29 = load float**, float*** %dat, align 8
  %30 = getelementptr inbounds float*, float** %29, i32 1
  %31 = load float*, float** %30, align 8
  %32 = getelementptr inbounds float, float* %31, i32 1
  %33 = load float, float* %32, align 4
  %34 = load float, float* %32, align 4
  %35 = fpext float %34 to double
  %36 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @2, i32 0, i32 0), double %35)
  %37 = load float**, float*** %dat, align 8
  %38 = getelementptr inbounds float*, float** %37, i32 2
  %39 = load float*, float** %38, align 8
  %40 = getelementptr inbounds float, float* %39, i32 2
  %41 = load float, float* %40, align 4
  %42 = load float, float* %40, align 4
  %43 = fpext float %42 to double
  %44 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @3, i32 0, i32 0), double %43)
  %45 = load float**, float*** %dat, align 8
  %46 = getelementptr inbounds float*, float** %45, i32 3
  %47 = load float*, float** %46, align 8
  %48 = getelementptr inbounds float, float* %47, i32 0
  %49 = load float, float* %48, align 4
  %50 = load float, float* %48, align 4
  %51 = fpext float %50 to double
  %52 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @4, i32 0, i32 0), double %51)
  %b = alloca %StdString.String**, align 8
  %53 = mul i32 8, 4
  %malloccall3 = tail call i8* @malloc(i32 %53)
  %54 = bitcast i8* %malloccall3 to %StdString.String**
  store %StdString.String** %54, %StdString.String*** %b, align 8
  %j04 = alloca i32, align 4
  store i32 0, i32* %j04, align 4
  %j15 = alloca i32, align 4
  store i32 0, i32* %j15, align 4
  br label %55

55:                                               ; preds = %57, %18
  %56 = load i32, i32* %j15, align 4
  %"j<sz6" = icmp slt i32 %56, 4
  br i1 %"j<sz6", label %57, label %68

57:                                               ; preds = %55
  %58 = load i32, i32* %j15, align 4
  %59 = mul i32 4, 16
  %malloccall7 = tail call i8* @malloc(i32 %59)
  %60 = bitcast i8* %malloccall7 to %StdString.String*
  %61 = load %StdString.String**, %StdString.String*** %b, align 8
  %62 = load i32, i32* %j15, align 4
  %63 = sext i32 %62 to i64
  %64 = getelementptr inbounds %StdString.String*, %StdString.String** %61, i64 %63
  %65 = load %StdString.String*, %StdString.String** %64, align 8
  store %StdString.String* %60, %StdString.String** %64, align 8
  %66 = load i32, i32* %j15, align 4
  %67 = add i32 %66, 1
  store i32 %67, i32* %j15, align 4
  br label %55

68:                                               ; preds = %55
  %69 = load %matrix4x4f.Matrix4x4f*, %matrix4x4f.Matrix4x4f** %m, align 8
  %70 = call %StdString.String** @"matrix4x4f.Matrix4x4f.test%matrix4x4f.Matrix4x4f*"(%matrix4x4f.Matrix4x4f* %69)
  store %StdString.String** %70, %StdString.String*** %b, align 8
  %71 = load %StdString.String**, %StdString.String*** %b, align 8
  %72 = getelementptr inbounds %StdString.String*, %StdString.String** %71, i32 2
  %73 = load %StdString.String*, %StdString.String** %72, align 8
  %74 = getelementptr inbounds %StdString.String, %StdString.String* %73, i32 1
  %75 = load %StdString.String, %StdString.String* %74, align 8
  %76 = call i8* @"StdString.String.toString%StdString.String*"(%StdString.String* %74)
  %77 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @5, i32 0, i32 0), i8* %76)
  %int = alloca i32, align 4
  store i32 255, i32* %int, align 4
  %char = alloca i8, align 1
  %78 = load i32, i32* %int, align 4
  %79 = sext i8 48 to i32
  %80 = add i32 %78, %79
  %81 = trunc i32 %80 to i8
  store i8 %81, i8* %char, align 1
  %82 = load i8, i8* %char, align 1
  %83 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @6, i32 0, i32 0), i8 %82)
  %str = alloca %StdString.String*, align 8
  %malloccall8 = tail call i8* @malloc(i32 16)
  %84 = bitcast i8* %malloccall8 to %StdString.String*
  store %StdString.String* %84, %StdString.String** %str, align 8
  %85 = load %StdString.String*, %StdString.String** %str, align 8
  call void @StdString.String._DefaultConstructor_(%StdString.String* %85)
  %86 = call %StdString.String* @StdString.IntToStringi32(i32 178)
  store %StdString.String* %86, %StdString.String** %str, align 8
  %87 = load %StdString.String*, %StdString.String** %str, align 8
  %88 = call i8* @"StdString.String.toString%StdString.String*"(%StdString.String* %87)
  %89 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @7, i32 0, i32 0), i8* %88)
  %90 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @9, i32 0, i32 0), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @8, i32 0, i32 0))
  %str2 = alloca %StdString.String*, align 8
  %malloccall9 = tail call i8* @malloc(i32 16)
  %91 = bitcast i8* %malloccall9 to %StdString.String*
  store %StdString.String* %91, %StdString.String** %str2, align 8
  %92 = load %StdString.String*, %StdString.String** %str2, align 8
  call void @StdString.String._DefaultConstructor_(%StdString.String* %92)
  %93 = call %StdString.String* @StdString.RealToStringdoublei32(double 0x406647F61672324D, i32 8)
  store %StdString.String* %93, %StdString.String** %str2, align 8
  %94 = load %StdString.String*, %StdString.String** %str2, align 8
  %95 = call i8* @"StdString.String.toString%StdString.String*"(%StdString.String* %94)
  %96 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @10, i32 0, i32 0), i8* %95)
  ret i32 0
}

attributes #0 = { cold noreturn nounwind }
