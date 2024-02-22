import StdMath;
import StdString;
module matrix4x4f
    public class Matrix4 inherits Object
        private field-array[16] float data;
        public method init(Matrix4 this)(in float n)
            let this.data[0] := n;
            let this.data[1] := 0.0f;
            let this.data[2] := 0.0f;
            let this.data[3] := 0.0f;
            let this.data[4] := 0.0f;
            let this.data[5] := n;
            let this.data[6] := 0.0f;
            let this.data[7] := 0.0f;
            let this.data[8] := 0.0f;
            let this.data[9] := 0.0f;
            let this.data[10] := n;
            let this.data[11] := 0.0f;
            let this.data[12] := 0.0f;
            let this.data[13] := 0.0f;
            let this.data[14] := 0.0f;
            let this.data[15] := n;
        end init;

        // TODO: parse multidimensional array returns
        public method rawData(Matrix4 this)(): array[] float
            return this.data;
        end rawData;

        public method multiplyMatrix(Matrix4 this)(in array[] float m)
            variable-array[16] float result;

            let result[0]  := this.data[0] * m[0] + this.data[1] * m[4] + this.data[2]  * m[8]  + this.data[3]  * m[12];
            let result[1]  := this.data[0] * m[1] + this.data[1] * m[5] + this.data[2]  * m[9]  + this.data[3]  * m[13];
            let result[2]  := this.data[0] * m[2] + this.data[1] * m[6] + this.data[2]  * m[10] + this.data[3]  * m[14];
            let result[3]  := this.data[0] * m[3] + this.data[1] * m[7] + this.data[2]  * m[11] + this.data[3]  * m[15];
            let result[4]  := this.data[4] * m[0] + this.data[5] * m[4] + this.data[6]  * m[8]  + this.data[7]  * m[12];
            let result[5]  := this.data[4] * m[1] + this.data[5] * m[5] + this.data[6]  * m[9]  + this.data[7]  * m[13];
            let result[6]  := this.data[4] * m[2] + this.data[5] * m[6] + this.data[6]  * m[10] + this.data[7]  * m[14];
            let result[7]  := this.data[4] * m[3] + this.data[5] * m[7] + this.data[6]  * m[11] + this.data[7]  * m[15];
            let result[8]  := this.data[8] * m[0] + this.data[9] * m[4] + this.data[10] * m[8]  + this.data[11] * m[12];
            let result[9]  := this.data[8] * m[1] + this.data[9] * m[5] + this.data[10] * m[9]  + this.data[11] * m[13];
            let result[10] := this.data[8] * m[2] + this.data[9] * m[6] + this.data[10] * m[10] + this.data[11] * m[14];
            let result[11] := this.data[8] * m[3] + this.data[9] * m[7] + this.data[10] * m[11] + this.data[11] * m[15];
            let result[12] := this.data[12] * m[0] + this.data[13] * m[4] + this.data[14] * m[8]  + this.data[15] * m[12];
            let result[13] := this.data[12] * m[1] + this.data[13] * m[5] + this.data[14] * m[9]  + this.data[15] * m[13];
            let result[14] := this.data[12] * m[2] + this.data[13] * m[6] + this.data[14] * m[10] + this.data[15] * m[14];
            let result[15] := this.data[12] * m[3] + this.data[13] * m[7] + this.data[14] * m[11] + this.data[15] * m[15];

            delete this.data;
            let this.data := result;
        end multiplyMatrix;


        public method rotate(Matrix4 this)(in float angle, in float x, in float y, in float z)
            // create 4x4 matrix rotation by x
            variable-float c := cos(angle);
            variable-float s := sin(angle);
            variable-float a := angle;

            variable-array[16] float temp;
            //let this.data := temp;
            let temp[0] := c + (1.0f - c) * x * x;
            let temp[1] := (1.0f - c) * x * y - s * z;
            let temp[2] := (1.0f - c) * x * z + s * y;
            let temp[3] := 0.0f;
            let temp[4] := (1.0f - c) * x * y + s * z;
            let temp[5] := c + (1.0f - c) * y * y;
            let temp[6] := (1.0f - c) * y * z - s * x;
            let temp[7] := 0.0f;
            let temp[8] := (1.0f - c) * x * z - s * y;
            let temp[9] := (1.0f - c) * y * z + s * x;
            let temp[10] := c + (1.0f - c) * z * z;
            let temp[11] := 0.0f;
            let temp[12] := 0.0f;
            let temp[13] := 0.0f;
            let temp[14] := 0.0f;
            let temp[15] := 1.0f;

            call this.multiplyMatrix(temp);
            delete temp;
        end rotate;

        // translate
        public method translate(Matrix4 this)(in float x, in float y, in float z)
            variable-array[16] float temp;
            let temp[0] := 1.0f;
            let temp[1] := 0.0f;
            let temp[2] := 0.0f;
            let temp[3] := 0.0f;
            let temp[4] := 0.0f;
            let temp[5] := 1.0f;
            let temp[6] := 0.0f;
            let temp[7] := 0.0f;
            let temp[8] := 0.0f;
            let temp[9] := 0.0f;
            let temp[10] := 1.0f;
            let temp[11] := 0.0f;
            let temp[12] := x;
            let temp[13] := y;
            let temp[14] := z;
            let temp[15] := 1.0f;
            call this.multiplyMatrix(temp);
            delete temp;
        end translate;


        // perspective
        public method perspective(Matrix4 this)(in float fov, in float aspect, in float near, in float far)
            variable-float f := 1.0f / tan(fov / 2.0f);
            let this.data[0] := f / aspect;
            let this.data[1] := 0.0f;
            let this.data[2] := 0.0f;
            let this.data[3] := 0.0f;
            let this.data[4] := 0.0f;
            let this.data[5] := f;
            let this.data[6] := 0.0f;
            let this.data[7] := 0.0f;
            let this.data[8] := 0.0f;
            let this.data[9] := 0.0f;
            let this.data[10] := (far + near) / (near - far);
            let this.data[11] := -1.0f;
            let this.data[12] := 0.0f;
            let this.data[13] := 0.0f;
            let this.data[14] := (2.0f * far * near) / (near - far);
            let this.data[15] := 0.0f;
        end perspective;
    end Matrix4;
start
end matrix4x4f.
