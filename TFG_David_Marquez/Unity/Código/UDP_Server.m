% Inicialización UDP, necesita un puerto para recibir y otro para enviar
% Se debe inicializar primero este script y luego ejecutar Unity
Server = udp('localhost', 9002, 'LocalPort', 9003);
fopen(Server);
disp(Server.status);

while(1)
   % Enviar mensaje
   % fwrite(t, msg); 
   % Recibir mensajes
   A = fread(Server, 10);
   if isempty(A) == 0
      B = char(A);
      disp(B);
   end
   if strcmp(B,'cierra') == 1
       print('Cerrando conexión');
       fclose(Server);
   end
end