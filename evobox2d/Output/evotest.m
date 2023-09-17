string = 'Org2018-4-17-0-5-51';

%Get all .csv files in this directory
d=dir(strcat(string, '*.csv'));
%d=dir('Org2018-4-17-0-5-51-3.csv');
L = length(d);

fprintf('Number of files: %d \n',L);

%Iterate through each file
for n=1:L
    
    fprintf('Name of file: %s \n', d(n).name);
    
    %Read in the file
    file = csvread(d(n).name);
    
    %Get the organism status at the beginning of the file
    orgStatus = file(1, 2);
    startingElement = 1;
    
    fprintf('Starting status: %d \n', orgStatus);
    
    %Iterate through each row
    for i=1:length(file(:, 2)) - 1
      
      %If the organism status has changed  
      if file(i, 2) ~= orgStatus || i == length(file(:, 2)) - 1
          orgStatus = file(i, 2);
          if file(i - 1, 2) == 0
            plot(file(startingElement : i - 1, 1), file(startingElement : i - 1, 3), 'g')
            
          elseif file(i - 1, 2) == 1
            plot(file(startingElement : i - 1, 1), file(startingElement : i - 1, 3), 'r')
            
          else
            plot(file(startingElement : i - 1, 1), file(startingElement : i - 1, 3), 'b')
             
          end
          startingElement = i;
              
      end
    end
    
    hold on
    
    
end






% A = csvread('Org2018-3-26-3-18-30-12.csv');
% B = csvread('Org2018-3-26-3-18-30-41.csv');
% 
% %Get the organism status at the beginning of the file
% orgStatus = A(1, 2);
% startingElement = 1;
% 
% for i=1:length(A(:, 2))
%   if A(i, 2) ~= orgStatus
%       orgStatus = A(i, 2);
%       if A(i - 1, 2) == 0
%         plot(A(startingElement : i - 1, 1), A(startingElement : i - 1, 3), 'g')
%         
%       elseif A(i - 1, 2) == 1
%         plot(A(startingElement : i - 1, 1), A(startingElement : i - 1, 3), 'r')
%         
%       else
%         plot(A(startingElement : i - 1, 1), A(startingElement : i - 1, 3), 'b')
%          
%       end
%       startingElement = i;
%   end
% end

%X = [1, 2, 3, 4]
%Y = [5, 6, 5, 4]
%plot(A(:, 1), A(:, 3), 'g')
%hold on
%plot(B(:, 1), B(:, 3), 'g')
%xlim([0 1000])
%ylim([0 300])
