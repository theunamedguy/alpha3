#include <platform.h>
#include <stdio.h>
bool kbd_supported(void)
{
  return true;
}
bool term_supported(void)
{
  return true;
}
bool file_supported(void)
{
  return true;
}
byte kbd_read(void)
{
  return getchar();
}
void term_write(byte c)
{
  putchar((char)c);
  fflush(stdout);
}
void file_open(alpha_ctx* ctx, const char* file)
{
  ctx->file_ptr=fopen(file, "rw+");
}
byte file_read(alpha_ctx* ctx)
{
  if(ctx->ports.file_open)
    {
      char c;
      fread(&c, 1, 1, (FILE*)ctx->file_ptr);
      return c;
    }
  return 0;
}
void file_write(alpha_ctx* ctx, byte b)
{
  if(ctx->ports.file_open)
    fwrite(&b, 1, 1, ctx->file_ptr);
  return;
} 
void file_close(alpha_ctx* ctx)
{
  fclose(ctx->file_ptr);
  return;
}
void mem_out_of_bounds(alpha_ctx* ctx)
{
  printf("Bad memory access at 0x%08X\n", ctx->regs[PC]);
}
