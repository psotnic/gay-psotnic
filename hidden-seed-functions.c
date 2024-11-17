void gen_cfg_seed(unsigned char *out)
{
    int i;
    out[0] = 157;
    out[1] = 222;
    out[2] = 239;
    out[3] = 175;
    out[4] = 111;
    out[5] = 142;
    out[6] = 155;
    out[7] = 213;
    out[8] = 197;
    out[9] = 197;
    out[10] = 209;
    out[11] = 117;
    out[12] = 24;
    out[13] = 9;
    out[14] = 162;
    out[15] = 125;

    for(i=0; i<16; ++i)
    {
        out[i] ^= 57;
    }
}

void gen_ul_seed(unsigned char *out)
{
    int i;
    out[0] = 151;
    out[1] = 30;
    out[2] = 178;
    out[3] = 7;
    out[4] = 184;
    out[5] = 171;
    out[6] = 6;
    out[7] = 160;
    out[8] = 248;
    out[9] = 188;
    out[10] = 87;
    out[11] = 6;
    out[12] = 162;
    out[13] = 114;
    out[14] = 46;
    out[15] = 83;

    for(i=0; i<16; ++i)
    {
        out[i] ^= 206;
    }
}

