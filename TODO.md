# TODO

- Reload not working right

- pipe2(fds, O_CLOEXEC | O_NONBLOCK);

- Error si user != Current y not root
- Mensaje de privilegios not drop pero no compruebo los programas, solo el global
- Si el user global es diferente al usuario actual y user de programa también, error.
- Si soy root, si user global es diferente y todos los programas también, no muestro mensaje, si global es igual a root (do not switch) y uno de los programas es también igual (do not switch) muestro warning
- Error si no root y user global es diferente o si uno de los programas es diferente
