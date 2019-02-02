function [] = Dibujar_Robot(Puntos_Moved_Red)
%figure

cla('reset');
hold ('on');
axis( 'equal');
axis([-inf inf -inf inf -300 900])
xlabel('x');
ylabel('y');
zlabel('z');
rotate3d( 'on');  %Rotación ON para app
view(45, 45);

for i=1:16
scatter3( Puntos_Moved_Red(:,1,i),Puntos_Moved_Red(:,2,i),Puntos_Moved_Red(:,3,i),4);
end


for i=2:2:14
X=[Puntos_Moved_Red(:,1,i)' Puntos_Moved_Red(1,1,i);Puntos_Moved_Red(:,1,i+1)' Puntos_Moved_Red(1,1,i+1)];
Y=[Puntos_Moved_Red(:,2,i)' Puntos_Moved_Red(1,2,i);Puntos_Moved_Red(:,2,i+1)' Puntos_Moved_Red(1,2,i+1)];
Z=[Puntos_Moved_Red(:,3,i)' Puntos_Moved_Red(1,3,i);Puntos_Moved_Red(:,3,i+1)' Puntos_Moved_Red(1,3,i+1)];
surf(X,Y,Z,'FaceColor',[1 0.84 0],'LineStyle','none');
end

for i=1:2:15
    for j=1:36
        line([Puntos_Moved_Red(j,1,i) Puntos_Moved_Red(j,1,i+1)],[Puntos_Moved_Red(j,2,i) Puntos_Moved_Red(j,2,i+1)],[Puntos_Moved_Red(j,3,i) Puntos_Moved_Red(j,3,i+1)]);
    end
end

end


