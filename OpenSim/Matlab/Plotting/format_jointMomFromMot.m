function [] = format_jointMomFromMot(fnames, timeRange, tInfo, ...
                subplotTitle, subplotAxisLabel, subplotRange, subplotTick)
% Purpose:  Formats the current figure window created by
%           compare_jointMomFromMot().
%
% Input:    fnames is a cell array of file names corresponding to 
%               the workflow-related motion files to be compared
%           timeRange.min and timeRange.max are the min and max values  
%               of the time axis, respectively
%           tInfo contains the following 'trial info', in addition to
%               other information:
%               *.mass - subject mass (used to scale axes for force data)
%           subplotTitle{} specifies title of each subplot
%           subplotAxisLabel{} specifies y-axis label of each subplot
%           subplotRange{} specifies the min, max values for scaling each
%               subplot
%           subplotTick{} specifies the y-tick labels of each subplot
%
% ASA, 11-05, revised 2-06


% Specify attributes of figure windows.
nPlotRows = 5;                      
nPlotCols = 2;
nSubPlots = nPlotRows * nPlotCols;  

% Specify attributes of all subplots.
tFontName = 'helvetica';            % attributes of subplot titles
tFontSize = 9;
tVerticalAlignment = 'middle';
aFontName = 'helvetica';            % attributes of axis labels and ticks
aFontSize = 8;
aTickDir = 'out';

% Specify time axis range and tick values.
xmin = timeRange.min;               
xmax = timeRange.max;
if xmax - xmin > 1.0
    xval = 0: 0.5: xmax;
elseif (xmax - xmin <= 1.0) && (xmax - xmin > 0.5)
    xval = 0: 0.2: xmax;
else
    xval = 0: 0.1: xmax;
end

% Specify y-axis limits and tick labels for vertical GRF data.
fmin = 0;                                
fmax = 1.5 * 9.8 * tInfo.mass;   
fval = 0:200:fmax;

% Format figure.
for plotIndex = 1:nSubPlots  
    subplot(nPlotRows, nPlotCols, plotIndex)
    hold on;
    
    if ~isempty(subplotTitle{plotIndex})
        title(subplotTitle{plotIndex});
            t = get(gca, 'title');
            set(t, 'FontName', tFontName, 'FontSize', tFontSize, ...
                'VerticalAlignment', tVerticalAlignment);
    end
    
    set(gca, 'xlim', [xmin xmax], 'xtick', xval, ...
            'FontName', aFontName, 'FontSize', aFontSize, ...
            'TickDir', aTickDir);
    if plotIndex == 1 || plotIndex == 2 || plotIndex == 9 || plotIndex == 10
        xlabel('time (s)');
            a = get(gca, 'xlabel');
            set(a, 'FontName', aFontName, 'FontSize', aFontSize);
    end
    
    if plotIndex == 1 || plotIndex == 2
        ymin = fmin;
        ymax = fmax;
        yval = fval;
        set(gca, 'ylim', [ymin ymax], 'ytick', yval, ...
               'FontName', aFontName, 'FontSize', aFontSize, ...
               'TickDir', aTickDir);
    elseif ~isempty(subplotRange{plotIndex})   
        ymin = subplotRange{plotIndex}(1);
        ymax = subplotRange{plotIndex}(2);
        if ~isempty(subplotTick{plotIndex})
            yval = subplotTick{plotIndex};
        else
            yval = ymin: round((ymax-ymin)/4) :ymax;
        end
        set(gca, 'ylim', [ymin ymax], 'ytick', yval, ...
               'FontName', aFontName, 'FontSize', aFontSize, ...
               'TickDir', aTickDir);
    else
        ytickpositions = get(gca, 'Ytick');
        if length(ytickpositions > 5)
            new_ytickpositions = ytickpositions(1:2:length(ytickpositions));
            set(gca, 'Ytick', new_ytickpositions);
        end  
    end
    
    if ~isempty(subplotAxisLabel{plotIndex})
        ylabel(subplotAxisLabel{plotIndex});
            a = get(gca, 'ylabel');
            set(a, 'FontName', aFontName, 'FontSize', aFontSize);   
    end
    set(gca, 'Box', 'off');
end

% Add legend.
subplot(nPlotRows, nPlotCols, 3);
set(gca, 'xlim', [0 1]);
styles = {'IK (thick cyan solid), RRA1 (thick cyan dotted), RRA2 (blue solid), CMC (blue dotted)', ...
          'red', 'cyan', 'magenta', 'green'};

legendString = cell( length(fnames) );
for i = 1:length(fnames)
    legendString{i} = [fnames{i}, ':  ', styles{i}];
end
legendString{i+1} = ' ';  
legendString{i+2} = 'Joint Moments from Inman et al. 1981: green curves';
legendString{i+3} = 'Joint Moments from Cappozzo et al. 1975: red curves';
legendString{i+4} = 'Joint Moments from Cappozzo 1983: thin cyan solid curves';
legendString{i+5} = 'Joint Moments from Crowninshield et al. 1978: yellow curves';
legendString{i+6} = 'Joint Moments from Patriarco et al. 1981: magenta curves';

for i = 1:length(legendString)
    x = 0;
    y = 1 - 0.15*i;
    text(x, y, legendString{i}, 'FontSize', 8, 'Interpreter', 'none');
end
axis off;
return;