; ModuleID = 'vec3f'
source_filename = "vec3f"
target triple = "x86_64-w64-windows-gnu"

%vec3f.Vec3f = type { float* }

@0 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@3 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@4 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@5 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@6 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@7 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@8 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1

declare i32 @printf(i8*, ...)

; Function Attrs: cold noreturn nounwind
declare void @llvm.trap() #0

declare void @GC_init()

declare i8* @GC_malloc(i32)

declare void @GC_free(i8*)

define void @vec3f.Vec3f._DefaultConstructor_(%vec3f.Vec3f* %0) {
vec3f.Vec3f_DefaultConstructor_Entry:
  %1 = alloca %vec3f.Vec3f*, align 8
  store %vec3f.Vec3f* %0, %vec3f.Vec3f** %1, align 8
  %2 = load %vec3f.Vec3f*, %vec3f.Vec3f** %1, align 8
  %3 = getelementptr inbounds %vec3f.Vec3f, %vec3f.Vec3f* %2, i32 0, i32 0
  %4 = mul i32 4, 3
  %malloccall = tail call i8* @malloc(i32 %4)
  %5 = bitcast i8* %malloccall to float*
  store float* %5, float** %3, align 8
  ret void
}

declare noalias i8* @malloc(i32)

define i8* @"vec3f.Vec3f.toString%vec3f.Vec3f*"(%vec3f.Vec3f* %varthis) {
vec3fvec3f.Vec3f.toStringEntry:
  %this = alloca %vec3f.Vec3f*, align 8
  store %vec3f.Vec3f* %varthis, %vec3f.Vec3f** %this, align 8
  call void @llvm.trap()
  unreachable
}

define void @"vec3f.Vec3f.init%vec3f.Vec3f*floatfloatfloat"(%vec3f.Vec3f* %outthis, float %inx, float %iny, float %inz) {
vec3fvec3f.Vec3f.initEntry:
  %this = alloca %vec3f.Vec3f*, align 8
  store %vec3f.Vec3f* %outthis, %vec3f.Vec3f** %this, align 8
  %x = alloca float, align 4
  store float %inx, float* %x, align 4
  %y = alloca float, align 4
  store float %iny, float* %y, align 4
  %z = alloca float, align 4
  store float %inz, float* %z, align 4
  %0 = load float, float* %x, align 4
  %1 = load %vec3f.Vec3f*, %vec3f.Vec3f** %this, align 8
  %2 = getelementptr inbounds %vec3f.Vec3f, %vec3f.Vec3f* %1, i32 0, i32 0
  %3 = load float*, float** %2, align 8
  %4 = getelementptr inbounds float, float* %3, i32 0
  store float %0, float* %4, align 4
  %5 = load float, float* %y, align 4
  %6 = load %vec3f.Vec3f*, %vec3f.Vec3f** %this, align 8
  %7 = getelementptr inbounds %vec3f.Vec3f, %vec3f.Vec3f* %6, i32 0, i32 0
  %8 = load float*, float** %7, align 8
  %9 = getelementptr inbounds float, float* %8, i32 1
  store float %5, float* %9, align 4
  %10 = load float, float* %z, align 4
  %11 = load %vec3f.Vec3f*, %vec3f.Vec3f** %this, align 8
  %12 = getelementptr inbounds %vec3f.Vec3f, %vec3f.Vec3f* %11, i32 0, i32 0
  %13 = load float*, float** %12, align 8
  %14 = getelementptr inbounds float, float* %13, i32 2
  store float %10, float* %14, align 4
  ret void
}

define void @"vec3f.Vec3f.init%vec3f.Vec3f*float"(%vec3f.Vec3f* %outthis, float %inn) {
vec3fvec3f.Vec3f.initEntry:
  %this = alloca %vec3f.Vec3f*, align 8
  store %vec3f.Vec3f* %outthis, %vec3f.Vec3f** %this, align 8
  %n = alloca float, align 4
  store float %inn, float* %n, align 4
  %0 = load float, float* %n, align 4
  %1 = load %vec3f.Vec3f*, %vec3f.Vec3f** %this, align 8
  %2 = getelementptr inbounds %vec3f.Vec3f, %vec3f.Vec3f* %1, i32 0, i32 0
  %3 = load float*, float** %2, align 8
  %4 = getelementptr inbounds float, float* %3, i32 0
  store float %0, float* %4, align 4
  %5 = load float, float* %n, align 4
  %6 = load %vec3f.Vec3f*, %vec3f.Vec3f** %this, align 8
  %7 = getelementptr inbounds %vec3f.Vec3f, %vec3f.Vec3f* %6, i32 0, i32 0
  %8 = load float*, float** %7, align 8
  %9 = getelementptr inbounds float, float* %8, i32 1
  store float %5, float* %9, align 4
  %10 = load float, float* %n, align 4
  %11 = load %vec3f.Vec3f*, %vec3f.Vec3f** %this, align 8
  %12 = getelementptr inbounds %vec3f.Vec3f, %vec3f.Vec3f* %11, i32 0, i32 0
  %13 = load float*, float** %12, align 8
  %14 = getelementptr inbounds float, float* %13, i32 2
  store float %10, float* %14, align 4
  ret void
}

define float @"vec3f.Vec3f.x%vec3f.Vec3f*"(%vec3f.Vec3f* %outthis) {
vec3fvec3f.Vec3f.xEntry:
  %this = alloca %vec3f.Vec3f*, align 8
  store %vec3f.Vec3f* %outthis, %vec3f.Vec3f** %this, align 8
  %0 = load %vec3f.Vec3f*, %vec3f.Vec3f** %this, align 8
  %1 = getelementptr inbounds %vec3f.Vec3f, %vec3f.Vec3f* %0, i32 0, i32 0
  %2 = load float*, float** %1, align 8
  %3 = getelementptr inbounds float, float* %2, i32 0
  %4 = load float, float* %3, align 4
  ret float %4
}

define float @"vec3f.Vec3f.y%vec3f.Vec3f*"(%vec3f.Vec3f* %outthis) {
vec3fvec3f.Vec3f.yEntry:
  %this = alloca %vec3f.Vec3f*, align 8
  store %vec3f.Vec3f* %outthis, %vec3f.Vec3f** %this, align 8
  %0 = load %vec3f.Vec3f*, %vec3f.Vec3f** %this, align 8
  %1 = getelementptr inbounds %vec3f.Vec3f, %vec3f.Vec3f* %0, i32 0, i32 0
  %2 = load float*, float** %1, align 8
  %3 = getelementptr inbounds float, float* %2, i32 1
  %4 = load float, float* %3, align 4
  ret float %4
}

define float @"vec3f.Vec3f.z%vec3f.Vec3f*"(%vec3f.Vec3f* %outthis) {
vec3fvec3f.Vec3f.zEntry:
  %this = alloca %vec3f.Vec3f*, align 8
  store %vec3f.Vec3f* %outthis, %vec3f.Vec3f** %this, align 8
  %0 = load %vec3f.Vec3f*, %vec3f.Vec3f** %this, align 8
  %1 = getelementptr inbounds %vec3f.Vec3f, %vec3f.Vec3f* %0, i32 0, i32 0
  %2 = load float*, float** %1, align 8
  %3 = getelementptr inbounds float, float* %2, i32 2
  %4 = load float, float* %3, align 4
  ret float %4
}

define void @"vec3f.Vec3f.setX%vec3f.Vec3f*float"(%vec3f.Vec3f* %outthis, float %inx) {
vec3fvec3f.Vec3f.setXEntry:
  %this = alloca %vec3f.Vec3f*, align 8
  store %vec3f.Vec3f* %outthis, %vec3f.Vec3f** %this, align 8
  %x = alloca float, align 4
  store float %inx, float* %x, align 4
  %0 = load float, float* %x, align 4
  %1 = load %vec3f.Vec3f*, %vec3f.Vec3f** %this, align 8
  %2 = getelementptr inbounds %vec3f.Vec3f, %vec3f.Vec3f* %1, i32 0, i32 0
  %3 = load float*, float** %2, align 8
  %4 = getelementptr inbounds float, float* %3, i32 0
  store float %0, float* %4, align 4
  ret void
}

define void @"vec3f.Vec3f.setY%vec3f.Vec3f*float"(%vec3f.Vec3f* %outthis, float %iny) {
vec3fvec3f.Vec3f.setYEntry:
  %this = alloca %vec3f.Vec3f*, align 8
  store %vec3f.Vec3f* %outthis, %vec3f.Vec3f** %this, align 8
  %y = alloca float, align 4
  store float %iny, float* %y, align 4
  %0 = load float, float* %y, align 4
  %1 = load %vec3f.Vec3f*, %vec3f.Vec3f** %this, align 8
  %2 = getelementptr inbounds %vec3f.Vec3f, %vec3f.Vec3f* %1, i32 0, i32 0
  %3 = load float*, float** %2, align 8
  %4 = getelementptr inbounds float, float* %3, i32 1
  store float %0, float* %4, align 4
  ret void
}

define void @"vec3f.Vec3f.setZ%vec3f.Vec3f*float"(%vec3f.Vec3f* %outthis, float %inz) {
vec3fvec3f.Vec3f.setZEntry:
  %this = alloca %vec3f.Vec3f*, align 8
  store %vec3f.Vec3f* %outthis, %vec3f.Vec3f** %this, align 8
  %z = alloca float, align 4
  store float %inz, float* %z, align 4
  %0 = load float, float* %z, align 4
  %1 = load %vec3f.Vec3f*, %vec3f.Vec3f** %this, align 8
  %2 = getelementptr inbounds %vec3f.Vec3f, %vec3f.Vec3f* %1, i32 0, i32 0
  %3 = load float*, float** %2, align 8
  %4 = getelementptr inbounds float, float* %3, i32 2
  store float %0, float* %4, align 4
  ret void
}

define float* @"vec3f.Vec3f.rawData%vec3f.Vec3f*"(%vec3f.Vec3f* %outthis) {
vec3fvec3f.Vec3f.rawDataEntry:
  %this = alloca %vec3f.Vec3f*, align 8
  store %vec3f.Vec3f* %outthis, %vec3f.Vec3f** %this, align 8
  %0 = load %vec3f.Vec3f*, %vec3f.Vec3f** %this, align 8
  %1 = getelementptr inbounds %vec3f.Vec3f, %vec3f.Vec3f* %0, i32 0, i32 0
  %2 = load float*, float** %1, align 8
  ret float* %2
}

define i32 @main() {
entry:
  call void @GC_init()
  %v = alloca %vec3f.Vec3f*, align 8
  %malloccall = tail call i8* @malloc(i32 8)
  %0 = bitcast i8* %malloccall to %vec3f.Vec3f*
  store %vec3f.Vec3f* %0, %vec3f.Vec3f** %v, align 8
  %1 = load %vec3f.Vec3f*, %vec3f.Vec3f** %v, align 8
  call void @vec3f.Vec3f._DefaultConstructor_(%vec3f.Vec3f* %1)
  %2 = load %vec3f.Vec3f*, %vec3f.Vec3f** %v, align 8
  call void @"vec3f.Vec3f.init%vec3f.Vec3f*floatfloatfloat"(%vec3f.Vec3f* %2, float 1.250000e+00, float 2.500000e+00, float 3.750000e+00)
  %3 = load %vec3f.Vec3f*, %vec3f.Vec3f** %v, align 8
  %4 = call float @"vec3f.Vec3f.x%vec3f.Vec3f*"(%vec3f.Vec3f* %3)
  %5 = fpext float %4 to double
  %6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), double %5)
  %7 = load %vec3f.Vec3f*, %vec3f.Vec3f** %v, align 8
  %8 = call float @"vec3f.Vec3f.y%vec3f.Vec3f*"(%vec3f.Vec3f* %7)
  %9 = fpext float %8 to double
  %10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @1, i32 0, i32 0), double %9)
  %11 = load %vec3f.Vec3f*, %vec3f.Vec3f** %v, align 8
  %12 = call float @"vec3f.Vec3f.z%vec3f.Vec3f*"(%vec3f.Vec3f* %11)
  %13 = fpext float %12 to double
  %14 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @2, i32 0, i32 0), double %13)
  %a = alloca float*, align 8
  %15 = mul i32 4, 3
  %malloccall1 = tail call i8* @malloc(i32 %15)
  %16 = bitcast i8* %malloccall1 to float*
  store float* %16, float** %a, align 8
  %17 = load %vec3f.Vec3f*, %vec3f.Vec3f** %v, align 8
  %18 = call float* @"vec3f.Vec3f.rawData%vec3f.Vec3f*"(%vec3f.Vec3f* %17)
  store float* %18, float** %a, align 8
  %19 = load float*, float** %a, align 8
  %20 = getelementptr inbounds float, float* %19, i32 0
  %21 = load float, float* %20, align 4
  %22 = fpext float %21 to double
  %23 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @3, i32 0, i32 0), double %22)
  %24 = load float*, float** %a, align 8
  %25 = getelementptr inbounds float, float* %24, i32 1
  %26 = load float, float* %25, align 4
  %27 = fpext float %26 to double
  %28 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @4, i32 0, i32 0), double %27)
  %29 = load float*, float** %a, align 8
  %30 = getelementptr inbounds float, float* %29, i32 2
  %31 = load float, float* %30, align 4
  %32 = fpext float %31 to double
  %33 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @5, i32 0, i32 0), double %32)
  %34 = load %vec3f.Vec3f*, %vec3f.Vec3f** %v, align 8
  call void @"vec3f.Vec3f.init%vec3f.Vec3f*float"(%vec3f.Vec3f* %34, float 0.000000e+00)
  %35 = load %vec3f.Vec3f*, %vec3f.Vec3f** %v, align 8
  %36 = call float @"vec3f.Vec3f.x%vec3f.Vec3f*"(%vec3f.Vec3f* %35)
  %37 = fpext float %36 to double
  %38 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @6, i32 0, i32 0), double %37)
  %39 = load %vec3f.Vec3f*, %vec3f.Vec3f** %v, align 8
  %40 = call float @"vec3f.Vec3f.y%vec3f.Vec3f*"(%vec3f.Vec3f* %39)
  %41 = fpext float %40 to double
  %42 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @7, i32 0, i32 0), double %41)
  %43 = load %vec3f.Vec3f*, %vec3f.Vec3f** %v, align 8
  %44 = call float @"vec3f.Vec3f.z%vec3f.Vec3f*"(%vec3f.Vec3f* %43)
  %45 = fpext float %44 to double
  %46 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @8, i32 0, i32 0), double %45)
  ret i32 0
}

attributes #0 = { cold noreturn nounwind }
