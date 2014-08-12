struct vec4 {
    float[4] v

    this(float x, float y, float z, float w) {
        .v = [x, y, z, w]
    }

    float x() return .v[0]
    float y() return .v[1]
    float z() return .v[2]
    float w() return .v[3]

    float dot(vec4 o) return .v[0] * o.v[0] + .v[1] * o.v[1] + .v[2] * o.v[2] + .v[3] * o.v[3]
    vec4 add(vec4 o) return vec4(.v[0] + o.v[0], .v[1] + o.v[1], .v[2] + o.v[2], .v[3] + o.v[3]);
}
