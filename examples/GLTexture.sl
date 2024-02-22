import glew;
import stb_image;
module GLTexture
    public class Texture inherits Object
        private field-array[256] character name;
        private field-array[0] character data;
        public field-integer texture;
        public method load(Texture this)(in array[] character filename)
            call stbi_set_flip_vertically_on_load(false);
            variable-integer width;
            variable-integer height;
            variable-integer channels;
            let this.data := stbi_load(filename, width, height, channels, 0);

            call glGenTextures(1, this.texture);
            call glBindTexture(3553, this.texture);
            call glTexImage2D(3553, 0, 6408, width, height, 0, 6408, 5121, this.data);
            call glTexParameteri(3553, 10242, 33071);
            call glTexParameteri(3553, 10243, 33071);
            call glTexParameteri(3553, 10240, 9729);
            call glTexParameteri(3553, 10241, 9729);
            call __glewGenerateMipmap(3553);
            call glBindTexture(3553, 0);

            call stbi_image_free(this.data);
        end load;

        public method bind(Texture this)()
            call glBindTexture(3553, this.texture);
        end bind;
    end Texture;

start
end GLTexture.