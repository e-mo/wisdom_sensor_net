#ifndef WISDOM_GATEWAY_MODULE_H
#define WISDOM_GATEWAY_MODULE_H

void gateway_init(void);
void gateway_start(void);
void gateway_send(void *data, uint size);

#endif // WISDOM_GATEWAY_MODULE_H
