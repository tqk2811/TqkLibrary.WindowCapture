using System;
using System.Windows.Input;
using System.Windows.Threading;
using TqkLibrary.WpfUi;

namespace WpfTest
{
    internal class BaseCommand : ICommand
    {
        protected readonly Dispatcher _dispatcher;
        protected BaseCommand() : this(App.Current.Dispatcher)
        {

        }
        protected BaseCommand(Dispatcher dispatcher)
        {
            this._dispatcher = dispatcher ?? throw new ArgumentNullException(nameof(dispatcher));
        }

        public BaseCommand(Action execute) : this(execute, () => true)
        {
        }
        public BaseCommand(Func<bool> canExecute) : this(() => { }, canExecute)
        {
        }
        public BaseCommand(Action execute, Func<bool> canExecute) : this(execute, canExecute, App.Current.Dispatcher)
        {
        }
        public BaseCommand(Action execute, Func<bool> canExecute, Dispatcher dispatcher)
        {
            this._execute = execute ?? throw new ArgumentNullException(nameof(execute));
            this._canExecute = canExecute ?? throw new ArgumentNullException(nameof(canExecute));
            this._dispatcher = dispatcher ?? throw new ArgumentNullException(nameof(dispatcher));
        }

        readonly Func<bool>? _canExecute;
        readonly Action? _execute;

        public virtual bool CanExecute(object? parameter)
        {
            return _canExecute?.Invoke() ?? true;
        }

        public virtual void Execute(object? parameter)
        {
            _execute?.Invoke();
        }

        public event EventHandler? CanExecuteChanged;
        public virtual void FireCanExecuteChanged(object? sender, EventArgs e)
        {
            _dispatcher.TrueThreadInvokeAsync(() => CanExecuteChanged?.Invoke(sender, e));
        }
        public virtual void FireCanExecuteChanged() => FireCanExecuteChanged(null, EventArgs.Empty);
    }
    internal class BaseCommand<TParam> : BaseCommand
    {
        public BaseCommand(Action<TParam> execute) : this(execute, (p) => true)
        {
        }
        public BaseCommand(Action<TParam> execute, Func<TParam, bool> canExecute)
        {
            this._execute = execute ?? throw new ArgumentNullException(nameof(execute));
            this._canExecute = canExecute ?? throw new ArgumentNullException(nameof(canExecute));
        }

        readonly Func<TParam, bool> _canExecute;
        readonly Action<TParam> _execute;

        public override bool CanExecute(object? parameter)
        {
            return _canExecute.Invoke((TParam)parameter!);
        }

        public override void Execute(object? parameter)
        {
            _execute?.Invoke((TParam)parameter!);
        }
    }
}
