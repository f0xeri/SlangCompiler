import stdstring;
module matrix4x4f
    public class Matrix4x4f inherits Object
        public field-array[4] array[4] float data;

        public method init(Matrix4x4f this)(in float n)
            let this.data[0][0] := n;
            let this.data[0][1] := 0.0f;
            let this.data[0][2] := 0.0f;
            let this.data[0][3] := 0.0f;
            let this.data[1][0] := 0.0f;
            let this.data[1][1] := n;
            let this.data[1][2] := 0.0f;
            let this.data[1][3] := 0.0f;
            let this.data[2][0] := 0.0f;
            let this.data[2][1] := 0.0f;
            let this.data[2][2] := n;
            let this.data[2][3] := 0.0f;
            let this.data[3][0] := 0.0f;
            let this.data[3][1] := 0.0f;
            let this.data[3][2] := 0.0f;
            let this.data[3][3] := n;
        end init;

        // TODO: parse multidimensional array returns
        public method rawData(Matrix4x4f this)(): array[] array[] float
            return this.data;
        end rawData;

        public method test(Matrix4x4f this)(): array[] array[] StdString.String
            variable-array[4] array[4] StdString.String aaa;
            call aaa[2][1].init("hello22");
            return aaa;
        end test;
    end Matrix4x4f;


start
    variable-Matrix4x4f m;
    call m.init(1.0f);
    variable-array[4] array[4] float dat;
    let dat := m.rawData();
    output dat[0][0];
    output dat[1][1];
    output dat[2][2];
    output dat[3][0];

    variable-array[4] array[4] StdString.String b;
    let b := m.test();
    output b[2][1].toString();
end matrix4x4f.
