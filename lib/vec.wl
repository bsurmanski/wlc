extern undecorated double sqrt(double x);

struct vec4 {
    float[4] v

    this(float x, float y, float z, float w) {
        .v = [x, y, z, w]
    }

    /*
    this(vec4 o) {
        .v = [o.x(), o.y(), o.z(), o.w()]
    }*/

    float get(int i) return .v[i]
    void set(int i, float val) .v[i] = val
    float x() return .v[0]
    float y() return .v[1]
    float z() return .v[2]
    float w() return .v[3]

    float dot(vec4 o) return .v[0] * o.v[0] + .v[1] * o.v[1] + .v[2] * o.v[2] + .v[3] * o.v[3]
    vec4 add(vec4 o) return vec4(.v[0] + o.v[0], .v[1] + o.v[1], .v[2] + o.v[2], .v[3] + o.v[3]);
    vec4 sub(vec4 o) return vec4(.v[0] - o.v[0], .v[1] - o.v[1], .v[2] - o.v[2], .v[3] - o.v[3]);
    vec4 mul(float f) return vec4(.v[0] * f, .v[1] * f, .v[2] * f, .v[3] * f);
    vec4 div(float f) return vec4(.v[0] / f, .v[1] / f, .v[2] / f, .v[3] / f);
    float lensq() return .v[0] * .v[0] + .v[1] * .v[1] + .v[2] * .v[2] + .v[3] * .v[3]
    float len() return sqrt(.lensq())
    vec4 normalized() return .div(.len())

    vec4 cross(vec4 o) {
        vec4 ret;
        ret.v[0] = .y() * o.z() - .z() * o.y()
        ret.v[1] = .z() * o.x() - .x() * o.z()
        ret.v[2] = .x() * o.y() - .y() * o.x()
        ret.v[3] = 0
        return ret
    }

    vec4 proj(vec4 o) {
        float numer = .dot(o)
        float denom = o.dot(o)
        return .mul(numer / denom)
    }

    vec4 orth(vec4 o) {
        vec4 r = .proj(o)
        return .sub(r)
    }
}

// row major matrix
struct mat4 {
    float[16] v

    this() {
        .v[0] = 1
        .v[1] = 0
        .v[2] = 0
        .v[3] = 0

        .v[4] = 0
        .v[5] = 1
        .v[6] = 0
        .v[7] = 0

        .v[8] = 0
        .v[9] = 0
        .v[10] = 1
        .v[11] = 0

        .v[12] = 0
        .v[13] = 0
        .v[14] = 0
        .v[15] = 1
    }

    float get(int i, int j) return .v[i+j*4]
    void set(int i, int j, float val) .v[i+j*4] = val

    mat4 mul(mat4 o) {
        mat4 ret
        for(int j = 0; j < 4; j++) {
            for(int i = 0; i < 4; i++) {
                ret.v[j*4+i] = 0.0f
                for(int k = 0; k < 4; k++) {
                    ret.v[j*4+i] += .v[i+k*4] * o.v[k+j*4]
                }
            }
        }
        return ret
    }

    vec4 vmul(vec4 o) {
        vec4 ret
        for(int j = 0; j < 4; j++) {
            ret.v[j] = 0.0f
            for(int i = 0; i < 4; i++) {
                ret.v[j] += .get(i, j) * o.get(j)
            }
        }
    }

    mat4 translate(vec4 o) {
        mat4 m = mat4()
        m.set(3,0, o.get(0))
        m.set(3,1, o.get(1))
        m.set(3,2, o.get(2))
        return m.mul(^this)
    }
}
