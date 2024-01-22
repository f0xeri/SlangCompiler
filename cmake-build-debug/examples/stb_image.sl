module stb_image
    public extern function stbi_load(in array[] character filename, out integer x, out integer y, out integer channels_in_file, in integer desired_channels): array[] character
    end stbi_load;

    public extern procedure stbi_image_free(in array[] character data)
    end stbi_image_free;

    public extern procedure stbi_set_flip_vertically_on_load(in boolean flag)
    end stbi_set_flip_vertically_on_load;
start

end stb_image.