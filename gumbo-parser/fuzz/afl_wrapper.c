#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include <gumbo.h>

int main(int argc, char *argv[])
{
  if (argc != 2)
    return 1;
  int fd = open(argv[1], O_RDONLY);
  if (fd < 0)
    return 2;

  size_t input_len = lseek(fd, 0, SEEK_END);
  (void)lseek(fd, 0, SEEK_SET);

  size_t page_mask = sysconf(_SC_PAGESIZE) - 1;
  size_t size = (input_len + page_mask) & ~page_mask;
  char *str = mmap(0, size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
  
  if (str == MAP_FAILED)
    return 3;
  GumboOutput *doc = gumbo_parse_with_options(&kGumboDefaultOptions, str,
                                              input_len);
  
  for (unsigned idx = 0; idx < doc->errors.length; ++idx)
  {
    GumboError *err = doc->errors.data[idx];
    char *error_text = 0;
    (void)gumbo_caret_diagnostic_to_string(err, str, input_len, &error_text);
    free(error_text);
  }

  gumbo_destroy_output(doc);

  // munmap(str, size);
  // close(fd);
  return 0;
}
