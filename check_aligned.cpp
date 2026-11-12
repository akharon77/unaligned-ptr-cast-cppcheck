int main() {
  char buf1[16] = {};
  auto *ptr1 = reinterpret_cast<uint32_t *>(buf1);
  (void)ptr1;

  alignas(uint32_t) char buf2[16] = {};
  auto *ptr2 = (uint32_t *)(buf2 + 4);
  (void)ptr2;

  auto *ptr3 = static_cast<uint32_t *>(static_cast<void *>(&buf2[0]));
  (void)ptr3;

  auto *ptr4 = static_cast<uint32_t *>(static_cast<void *>(&buf2[1]));
  (void)ptr4;

  auto *ptr5 = reinterpret_cast<const uint32_t *>(buf1);
  (void)ptr5;

  uint32_t buf3[16] = {};
  auto *ptr6 = reinterpret_cast<uint64_t*>(buf3);
  (void)ptr6;

  return 0;
}
