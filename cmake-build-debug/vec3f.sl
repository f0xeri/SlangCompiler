module vec3f
    public class Vec3f inherits Object
        public field-array[3] float data;

        public method init(Vec3f this)(in float x, in float y, in float z)
            let this.data[0] := x;
            let this.data[1] := y;
            let this.data[2] := z;
        end init;

        public method init(Vec3f this)(in float n)
            let this.data[0] := n;
            let this.data[1] := n;
            let this.data[2] := n;
        end init;

        public method x(Vec3f this)(): float
            return this.data[0];
        end x;

        public method y(Vec3f this)(): float
            return this.data[1];
        end y;

        public method z(Vec3f this)(): float
            return this.data[2];
        end z;

        public method setX(Vec3f this)(in float x)
            let this.data[0] := x;
        end setX;

        public method setY(Vec3f this)(in float y)
            let this.data[1] := y;
        end setY;

        public method setZ(Vec3f this)(in float z)
            let this.data[2] := z;
        end setZ;

        public method rawData(Vec3f this)(): array[] float
            return this.data;
        end rawData;
    end Vec3f;
start
    variable-Vec3f v;
    call v.init(1.25f, 2.5f, 3.75f);
    output v.x();
    output v.y();
    output v.z();
    variable-array[3] float a;
    let a := v.rawData();
    output a[0];
    output a[1];
    output a[2];
    call v.init(0.0f);
    output v.x();
    output v.y();
    output v.z();
end vec3f.
